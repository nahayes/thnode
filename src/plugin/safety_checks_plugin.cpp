/*
Copyright 2026 the Thnode Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <fstream>
#include <google/protobuf/text_format.h>
#include <optional>
#include <unordered_set>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include "ast_visitor.h"
#include "rewrite_manager.h"

#include "proto/config.pb.h"

using namespace clang;
using namespace std;

class SafetyCheckConsumer : public ASTConsumer {
private:
  AstVisitor Visitor_;

public:
  explicit SafetyCheckConsumer(ASTContext *Context, RewriteManager &R,
                               const thnode::Config &Config)
      : Visitor_(Context, R, Config) {}

  virtual void HandleTranslationUnit(ASTContext &Context) {
    Visitor_.TraverseDecl(Context.getTranslationUnitDecl());
  }
};

class SafetyCheckAction : public PluginASTAction {
private:
  std::unique_ptr<Rewriter> Rewriter_;
  std::unique_ptr<RewriteManager> RewriteManager_;
  thnode::Config Config_;
  std::optional<std::string> OutputPath_;
  std::optional<std::string> AddInclude_;

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef File) override {
    Rewriter_ =
        std::make_unique<Rewriter>(CI.getSourceManager(), CI.getLangOpts());
    RewriteManager_ = std::make_unique<RewriteManager>(*Rewriter_);
    return std::make_unique<SafetyCheckConsumer>(&CI.getASTContext(),
                                                 *RewriteManager_, Config_);
  }

  static bool applyConfigString(thnode::Config *Config,
                                const std::string &ConfigString) {
    if (!google::protobuf::TextFormat::ParseFromString(ConfigString, Config)) {
      llvm::errs() << "ERROR: safty-check plugin: Failed to parse message from "
                      "text format; message string was: "
                   << ConfigString << "\n";
      return false;
    }

    return true;
  }

  static bool applyConfigFile(thnode::Config *Config,
                              const std::string &ConfigPath) {
    std::ifstream File(ConfigPath);
    if (!File) {
      llvm::errs() << "ERROR: safty-check plugin: Failed to open config file: "
                   << ConfigPath << "\n";
      return false;
    }

    std::string ConfigString((std::istreambuf_iterator<char>(File)),
                             std::istreambuf_iterator<char>());

    return applyConfigString(Config, ConfigString);
  }

  // Parse the given plugin command line arguments.
  // Called by clang once before the AST is built.
  //
  // From https://clang.llvm.org/doxygen/FrontendAction_8h_source.html :
  //
  // \param CI - The compiler instance, for use in reporting diagnostics.
  // \return True if the parsing succeeded; otherwise the plugin will be
  // destroyed and no action run. The plugin is responsible for using the
  // CompilerInstance's Diagnostic object to report errors.
  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    (void)CI;
    GOOGLE_PROTOBUF_VERIFY_VERSION; // Initialize protocol buffer support.

    const char *ThnodeConfig = std::getenv("THNODE_CONFIG");
    if (ThnodeConfig != nullptr) {
      if (!applyConfigString(&Config_, std::string(ThnodeConfig))) {
        llvm::errs()
            << "ERROR: safty-check plugin: Failed to parse config string: "
            << "\"" << ThnodeConfig << "\""
            << "\n";
        return false;
      }
    }

    for (size_t I = 0; I < Args.size(); ++I) {
      const std::string &Arg = Args[I];

      if (Arg == "-o") {
        if (I + 1 >= Args.size()) {
          llvm::errs()
              << "ERROR: safty-check plugin: -o requires a path argument\n";
          return false;
        }
        OutputPath_ = Args[++I];
        continue;
      }

      if (Arg == "-add_include") {
        if (I + 1 >= Args.size()) {
          llvm::errs()
              << "ERROR: safty-check plugin: -o requires a path argument\n";
          return false;
        }
        AddInclude_ = Args[++I];
        continue;
      }

      size_t EqualsPos = Arg.find('=');
      if (Arg.compare(0, 2, "--") != 0 || EqualsPos == std::string::npos) {
        llvm::errs() << "arguments to safty-check plugin must be of the form "
                        "--<key>=<value>; got '"
                     << Arg << "'\n";
        return false;
      }

      // Split arg into "--<key>=<value>".
      std::string Key = Arg.substr(2, EqualsPos - 2);
      std::string Value = Arg.substr(EqualsPos + 1);

      if (Key == "config-file") {
        if (!applyConfigFile(&Config_, Value)) {
          return false;
        }
      } else if (Key == "config-string") {
        if (!applyConfigString(&Config_, Value)) {
          return false;
        };
      } else {
        llvm::errs() << "ERROR: safty-check plugin: unknown argument: " << Arg
                     << "\n";
        return false;
      }
    }
    return true;
  }

  void EndSourceFileAction() override {
    SourceManager &SM = Rewriter_->getSourceMgr();
    // If OutputPath is not set, write the result to stdout.
    if (!OutputPath_.has_value()) {
      Rewriter_->getEditBuffer(SM.getMainFileID()).write(llvm::outs());
      return;
    }

    std::error_code Ec;
    llvm::raw_fd_ostream OutFile(OutputPath_.value(), Ec,
                                 llvm::sys::fs::OF_Text);
    if (Ec) {
      llvm::errs() << "ERROR: safty-check plugin: Failed to open output file '"
                   << OutputPath_.value() << "': " << Ec.message() << "\n";
      return;
    }

    if (AddInclude_.has_value()) {
      OutFile << "#include \"" << AddInclude_.value() << "\"\n\n";
    }
    Rewriter_->getEditBuffer(SM.getMainFileID()).write(OutFile);
  }
};

static FrontendPluginRegistry::Add<SafetyCheckAction>
    X("safety-checks", "Add checks for unsafe code");
