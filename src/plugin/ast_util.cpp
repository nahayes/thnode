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

#include "ast_util.h"

#include "clang/AST/AST.h"
#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/ParentMapContext.h"

namespace ast_util {

const char *whyMustExprBeConstantToString(WhyMustExprBeConstantEnum Reason) {
  switch (Reason) {
  case WhyMustExprBeConstantEnum::NoReason:
    return "NoReason";
  case WhyMustExprBeConstantEnum::IsConstInitializer:
    return "IsConstInitializer";
  case WhyMustExprBeConstantEnum::IsConstExprExpression:
    return "IsConstExprExpression";
  default:
    return "INVALID!";
  }
}

WhyMustExprBeConstantEnum mustExprBeConstant(clang::ASTContext &Ctx,
                                             clang::Expr *E) {
  clang::DynTypedNode Node = clang::DynTypedNode::create(*E);

  clang::Expr *OutermostExpr = E;
  while (true) {
    auto Parents = Ctx.getParents(Node);
    if (Parents.empty()) {
      return WhyMustExprBeConstantEnum::NoReason;
    }
    Node = Parents[0];

    // Still climbing inside an expression.
    if (auto *E = Node.get<clang::Expr>()) {
      OutermostExpr = const_cast<clang::Expr *>(E);
      continue;
    }

    if (auto *VD = Node.get<clang::VarDecl>()) {
      if (VD->hasGlobalStorage()) {
        // Initializer must be constant.
        return WhyMustExprBeConstantEnum::IsConstInitializer;
      }

      if (VD->isConstexpr() && VD->getInit() == OutermostExpr) {
        // constexpr int i = 123;
        return WhyMustExprBeConstantEnum::IsConstExprExpression;
      }
    }

    // TODO: There are many more reasons an expression might need to be
    // constant. Examples include the size of a bit field, the value of a case
    // label, the value of an enum, ...
  }
}

} // namespace ast_util
