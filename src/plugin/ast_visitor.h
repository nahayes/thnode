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

#include <unordered_set>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "proto/config.pb.h"
#include "rewrite_manager.h"

class AstVisitor : public clang::RecursiveASTVisitor<AstVisitor> {
public:
  explicit AstVisitor(clang::ASTContext *Ctx, RewriteManager &R,
                      const thnode::Config &Config)
      : Context_(Ctx), Rewriter_(R), Config_(Config) {}

  // Visitor methods:
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE);
  bool VisitBinaryOperator(clang::BinaryOperator *OP);
  bool VisitMemberExpr(clang::MemberExpr *ME);
  bool VisitUnaryOperator(clang::UnaryOperator *OP);
  bool VisitCallExpr(clang::CallExpr *CE);

  // Traverse post-order to ensure we visit the children first.
  // This is necessary to work with nested operators: `*(os.isp).intp`
  bool shouldTraversePostOrder() const { return true; }

private:
  bool shouldWrapCallTo(const std::string &FunName);

  clang::ASTContext *Context_;
  RewriteManager &Rewriter_;
  const thnode::Config &Config_;
};
