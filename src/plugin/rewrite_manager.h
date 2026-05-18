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

#pragma once

#include <string>

#include "third_party/expected/include/tl/expected.hpp"

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "code_location.h"

enum class RewriteError {
  KReplacementFailed = 1,
};

// Wrapper around the rewriter; checks for duplicate changes.
class RewriteManager {
public:
  explicit RewriteManager(clang::Rewriter &Rewriter) : Rewriter_(Rewriter) {}

  clang::SourceManager &getSourceMgr() { return Rewriter_.getSourceMgr(); }
  const clang::LangOptions &getLangOpts() { return Rewriter_.getLangOpts(); }

  tl::expected<std::string, RewriteError>
  getRewrittenText(clang::CharSourceRange Range) {
    const auto Result = Rewriter_.getRewrittenText(Range);

    // If the start or end of the range was unrewritable or if they are
    // in different buffers, this returns an empty string.
    if (Result.empty()) {
      return tl::unexpected(RewriteError::KReplacementFailed);
    }

    return Result;
  }

  tl::expected<std::string, RewriteError>
  getRewrittenText(clang::SourceRange Range) {
    const auto Result = Rewriter_.getRewrittenText(Range);

    // If the start or end of the range was unrewritable or if they are
    // in different buffers, this returns an empty string.
    if (Result.empty()) {
      return tl::unexpected(RewriteError::KReplacementFailed);
    }

    return Result;
  }

  tl::expected<std::string, RewriteError>
  getRewrittenTextAdjustEnding(clang::SourceRange Range) {
    // The range of an expression may have changed with previous edits.
    // Need to find the new end...
    //
    // https://stackoverflow.com/questions/78457009/using-clangs-libtooling-to-rewrite-nested-ternary-expressions

    const auto &SM = getSourceMgr();
    const auto &LO = getLangOpts();

    // Why don't we need to move the start?
    // My guess: We always edit from the end of the file to the front.
    // The start of an expression can't have moved (yet).
    // TODO: Read more to understand this.
    const auto StartLoc = Range.getBegin();
    const auto EndLoc =
        clang::Lexer::getLocForEndOfToken(Range.getEnd(), 0, SM, LO);

    if (!SM.isWrittenInSameFile(StartLoc, EndLoc)) {
      // Empty string signals failure.
      // This can happen if a macro is in the range.
      // See infback.c in zlib, macro BITS:
      //
      // line 388: lencode[BITS(state->lenbits)];
      return tl::unexpected(RewriteError::KReplacementFailed);
    }

    const auto UpdatedRange =
        clang::CharSourceRange::getCharRange(StartLoc, EndLoc);

    const auto Result = Rewriter_.getRewrittenText(UpdatedRange);
    if (Result.empty()) {
      return tl::unexpected(RewriteError::KReplacementFailed);
    }
    return Result;
  }

  void replaceText(const CodeLocation &Caller, clang::SourceRange Range,
                   llvm::StringRef Txt) {
    bool ReplaceFailed = Rewriter_.ReplaceText(Range, Txt);
    if (ReplaceFailed) {
      llvm::errs() << "RewriteManager; FALSE " << Caller
                   << ": Failed to replace text in range ["
                   << Range.getBegin().printToString(Rewriter_.getSourceMgr())
                   << ", "
                   << Range.getEnd().printToString(Rewriter_.getSourceMgr())
                   << "]\n";
      exit(1);
    }
  }

  void insertText(const CodeLocation &Caller, clang::SourceLocation Loc,
                  clang::StringRef Str, bool InsertSpace = true,
                  bool IndentNewLines = false) {
    llvm::errs() << "InsertText; " << Caller << ": Inserting text at location ["
                 << Loc.printToString(Rewriter_.getSourceMgr())
                 << "] with: " << Str << "\n";
    bool InsertFailed =
        Rewriter_.InsertText(Loc, Str, InsertSpace, IndentNewLines);
    if (InsertFailed) {
      llvm::errs() << "RewriteManager; FALSE " << Caller
                   << ": Failed to insert text at location ["
                   << Loc.printToString(Rewriter_.getSourceMgr()) << "]\n";
      exit(1);
    }
  }

  void insertTextAfter(const CodeLocation &Caller, clang::SourceLocation Loc,
                       clang::StringRef Str) {
    bool InsertFailed = Rewriter_.InsertTextAfterToken(Loc, Str);
    if (InsertFailed) {
      llvm::errs() << "RewriteManager; FALSE " << Caller
                   << ": Failed to insert text at location ["
                   << Loc.printToString(Rewriter_.getSourceMgr()) << "]\n";
      exit(1);
    }
  }

  void insertTextAtEndOfLine(const CodeLocation &Caller,
                             clang::SourceLocation Loc, clang::StringRef Str) {
    clang::SourceManager &SM = Rewriter_.getSourceMgr();

    // Presumed location has the line number.
    clang::PresumedLoc Ploc = SM.getPresumedLoc(Loc);
    if (Ploc.isInvalid()) {
      llvm::errs() << "RewriteManager; " << Caller << ": ploc is invalid at ["
                   << Loc.printToString(Rewriter_.getSourceMgr()) << "]\n";
      exit(1);
    }

    const auto FileId = SM.getFileID(Loc);
    const auto Line = Ploc.getLine();
    const int MaxLineLen = 100000;
    clang::SourceLocation EndOfLineLoc =
    SM.translateLineCol(FileId, Line, MaxLineLen);

    Rewriter_.InsertText(EndOfLineLoc, Str, true, true);
  }

protected:
  clang::Rewriter &Rewriter_; // Not owned. Lifetime is managed by user.
};
