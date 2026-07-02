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

#include "ast_visitor.h"
#include "ast_util.h"

#include <regex>

#include "third_party/expected/include/tl/expected.hpp"

// Given a type, get the widest version (e.g., int -> intmax_t, unsigned ->
// uintmax_t, float -> long double).
// https://en.cppreference.com/w/c/language/generic.html
static std::string getMaxWidthVersionOfType(clang::ASTContext *Context,
                                            clang::QualType InputQualType) {
  const clang::Type *InputType = InputQualType.getTypePtr();

  if (InputType->isUnsignedIntegerType()) {
    return "uintmax_t";
  }

  if (InputType->isSignedIntegerType()) {
    return "intmax_t";
  }

  if (InputType->isFloatingType()) {
    // For floating types, use long double.
    return "long double";
  }

  // Fallback to the original type.
  return InputQualType.getAsString();
}

bool AstVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE) {
  clang::SourceManager &SM = Context_->getSourceManager();

  if (!Config_.dont_limit_to_main_file()) {
    // If this is not the main file the clang plugin is operating on, return
    // true.

    const clang::FileEntry *MainFile = SM.getFileEntryForID(SM.getMainFileID());
    const clang::FileEntry *ExprFile =
        SM.getFileEntryForID(SM.getFileID(ASE->getExprLoc()));
    if (ExprFile != MainFile) {
      return true;
    }
  }

  // Goal: $A[$B] -> ARRAY_ACCESS($A, $B)

  // Get the base array expression (left-hand side)
  clang::Expr *ArrayExpr = ASE->getBase();
  const auto ArrayStr =
      Rewriter_.getRewrittenTextAdjustEnding(ArrayExpr->getSourceRange());
  if (!ArrayStr.has_value()) {
    return true;
  }

  // Get the index expression (right-hand side)
  clang::Expr *IndexExpr = ASE->getIdx();
  const auto IndexStr =
      Rewriter_.getRewrittenTextAdjustEnding(IndexExpr->getSourceRange());
  if (!IndexStr.has_value()) {
    return true;
  }

  auto ReasonExprMustBeConst =
      ast_util::mustExprBeConstant(*Context_, IndexExpr);
  if (ReasonExprMustBeConst != ast_util::WhyMustExprBeConstantEnum::NoReason) {
    // Say why by adding to eol...
    auto EndLoc = IndexExpr->getEndLoc();

    std::string Reason =
        std::string(" // ARRAY-SKIPPED:") +
        ast_util::whyMustExprBeConstantToString(ReasonExprMustBeConst);

    Rewriter_.insertTextAtEndOfLine(HERE, EndLoc, Reason);
    return true;
  }

  Rewriter_.replaceText(HERE, ASE->getSourceRange(),
                        std::string("ARRAY_ACCESS(") + ArrayStr.value() + ", " +
                            IndexStr.value() + ")");

  return true;
}

bool AstVisitor::shouldWrapCallTo(const std::string &FunName) {
  // See if a function name should be wrapped by the CALL macro,
  // according to the patterns in config_.wrap_call_re().
  for (const auto &Pattern : Config_.wrap_call_re()) {
    std::regex Re(Pattern);
    if (std::regex_match(FunName, Re)) {
      return true;
    }
  }
  return false;
}

bool AstVisitor::VisitCallExpr(clang::CallExpr *Call) {
  // Only rewrite calls in the main file unless configured otherwise.
  if (!Config_.dont_limit_to_main_file()) {
    clang::SourceManager &SM = Context_->getSourceManager();
    const clang::FileEntry *MainFile = SM.getFileEntryForID(SM.getMainFileID());
    const clang::FileEntry *ExprFile =
        SM.getFileEntryForID(SM.getFileID(Call->getExprLoc()));
    if (ExprFile != MainFile) {
      return true;
    }
  }

  clang::Expr *Callee = Call->getCallee()->IgnoreParenImpCasts();
  const clang::FunctionDecl *FD = nullptr;

  if (auto *DRE = llvm::dyn_cast<clang::DeclRefExpr>(Callee)) {
    FD = llvm::dyn_cast<clang::FunctionDecl>(DRE->getDecl());
    if (FD && shouldWrapCallTo(FD->getNameAsString())) {
      // Get the rewritten text for the callee and arguments
      auto CalleeStr =
          Rewriter_.getRewrittenTextAdjustEnding(Callee->getSourceRange());
      if (!CalleeStr.has_value()) {
        return true; // Bail if can't get rewritten text
      }

      auto FunNameText =
          Rewriter_.getRewrittenTextAdjustEnding(Callee->getSourceRange());
      if (!FunNameText.has_value()) {
        // Bail if can't get arguments' text
        return true;
      }

      std::string Replacement = std::string("CALL(") + FunNameText.value() +
                                ")"; //  + callArgsText.value();

      Rewriter_.replaceText(HERE, Callee->getSourceRange(), Replacement);

      return true;
    }
  }

  return true;
}

bool AstVisitor::VisitBinaryOperator(clang::BinaryOperator *OP) {
  if (!Config_.dont_limit_to_main_file()) {
    // Is this is the main file the clang plugin is OPerating on?
    // If not, return to avoid editing it.
    clang::SourceManager &SM = Context_->getSourceManager();
    const clang::FileEntry *MainFile = SM.getFileEntryForID(SM.getMainFileID());
    const clang::FileEntry *ExprFile =
        SM.getFileEntryForID(SM.getFileID(OP->getExprLoc()));
    if (ExprFile != MainFile) {
      return true;
    }
  }

  // Handle division and modulo operators.
  if (OP->getOpcode() == clang::BO_Div ||
      OP->getOpcode() == clang::BO_DivAssign ||
      OP->getOpcode() == clang::BO_Rem ||
      OP->getOpcode() == clang::BO_RemAssign) {
    auto *Denominator = OP->getRHS();

    auto DenomBegin = Denominator->getBeginLoc();
    auto DenomEnd = Denominator->getEndLoc();

    // If either the start or end of the denominator is in a macro, skip
    // instrumentation.
    if (DenomBegin.isMacroID() || DenomEnd.isMacroID()) {
      // Altering text within a macro expansion does not work. Replacements
      // will fail to apply.

      clang::DiagnosticsEngine &DE = Context_->getDiagnostics();
      const unsigned ID =
          DE.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                             "Thnode-error: Divisor is built with a macro.");

      // Report a warning at the denominator's source range.
      clang::SourceRange DenomRange(DenomBegin, DenomEnd);
      DE.Report(DenomBegin, ID) << DenomRange;

      return true;
    }

    clang::Expr *OPAsExpr = OP;
    auto ReasonExprMustBeConst =
        ast_util::mustExprBeConstant(*Context_, OPAsExpr);
    if (ReasonExprMustBeConst !=
        ast_util::WhyMustExprBeConstantEnum::NoReason) {
      // Say why by adding a comment.
      auto EndLoc = OPAsExpr->getEndLoc();
      std::string Reason =
          std::string(" // DENOM-SKIPPED:") +
          ast_util::whyMustExprBeConstantToString(ReasonExprMustBeConst);
      Rewriter_.insertTextAtEndOfLine(HERE, EndLoc, Reason);
      return true;
    }

    // Get the original denominator expression.
    const auto OriginalDenomExpr =
        Rewriter_.getRewrittenText(Denominator->getSourceRange());
    if (!OriginalDenomExpr.has_value()) {
      return true;
    }

    // Get the type of the denominator.
    // We widen the type so that one handler can cover all unsigned integer
    // types (unsigned char, unsigned short, unsigned int, ...), a second
    // handler can cover all signed integer types, etc.
    clang::QualType DenomType = Denominator->getType();
    const std::string ExprType = getMaxWidthVersionOfType(Context_, DenomType);

    // Type long double has a space. Turn it to a '_'.
    // This allows macros to construct a function name.
    std::string ExprTypeNoSpace = llvm::StringRef(ExprType).str();
    std::replace(ExprTypeNoSpace.begin(), ExprTypeNoSpace.end(), ' ', '_');

    std::string NewDenomExpr = std::string("DENOM_CHECK( (") +
                               OriginalDenomExpr.value() + "), " +
                               ExprTypeNoSpace + ")";

    // Instrument the denominator with our safety check.
    const auto Expr = Rewriter_.getRewrittenText(Denominator->getSourceRange());
    if (!Expr.has_value()) {
      return true;
    }
    Rewriter_.replaceText(HERE, Denominator->getSourceRange(), NewDenomExpr);
  }

  return true;
}

bool AstVisitor::VisitMemberExpr(clang::MemberExpr *ME) {
  if (!Config_.dont_limit_to_main_file()) {
    // If this is not the main file the clang plugin is operating on, return
    // true
    clang::SourceManager &SM = Context_->getSourceManager();
    const clang::FileEntry *MainFile = SM.getFileEntryForID(SM.getMainFileID());
    const clang::FileEntry *ExprFile =
        SM.getFileEntryForID(SM.getFileID(ME->getExprLoc()));
    if (ExprFile != MainFile) {
      return true;
    }
  }

  if (ME->isArrow()) {
    auto ReasonExprMustBeConst = ast_util::mustExprBeConstant(*Context_, ME);
    if (ReasonExprMustBeConst !=
        ast_util::WhyMustExprBeConstantEnum::NoReason) {
      auto EndLoc = ME->getEndLoc();
      std::string Reason =
          std::string(" // NULLCHECK-SKIPPED:") +
          ast_util::whyMustExprBeConstantToString(ReasonExprMustBeConst);
      Rewriter_.insertTextAtEndOfLine(HERE, EndLoc, Reason);
      return true;
    }

    // Get the expression being dereferenced (without the '->').
    const auto Expr =
        Rewriter_.getRewrittenText(ME->getBase()->getSourceRange());
    if (!Expr.has_value()) {
      return true;
    }

    // Get the type of the pointer being dereferenced.
    clang::QualType PtrType = ME->getBase()->getType();
    std::string ExprType = PtrType.getAsString();

    // Build the new expression with the runtime check.
    std::string NewExpr =
        std::string("NULL_CHECK( (" + Expr.value() + "), " + ExprType + ")");

    // Replace the original expression.
    Rewriter_.replaceText(HERE, ME->getBase()->getSourceRange(), NewExpr);
  }

  return true;
}

bool AstVisitor::VisitUnaryOperator(clang::UnaryOperator *OP) {
  if (!Config_.dont_limit_to_main_file()) {
    // If this is not the main file the clang plugin is operating on, return
    // true.
    clang::SourceManager &SM = Context_->getSourceManager();
    const clang::FileEntry *MainFile = SM.getFileEntryForID(SM.getMainFileID());
    const clang::FileEntry *ExprFile =
        SM.getFileEntryForID(SM.getFileID(OP->getExprLoc()));
    if (ExprFile != MainFile) {
      return true;
    }
  }

  if (OP->getOpcode() == clang::UO_Deref) {
    // Goal: *P -> *(NULL_CHECK(P, PType))

    // Get the text of the expression being dereferenced (without the '*').
    const auto ExprText =
        Rewriter_.getRewrittenText(OP->getSubExpr()->getSourceRange());
    if (!ExprText.has_value()) {
      return true;
    }

    auto ReasonExprMustBeConst = ast_util::mustExprBeConstant(*Context_, OP);
    if (ReasonExprMustBeConst !=
        ast_util::WhyMustExprBeConstantEnum::NoReason) {
      auto EndLoc = OP->getEndLoc();
      std::string Reason =
          std::string(" // NULLCHECK-SKIPPED:") +
          ast_util::whyMustExprBeConstantToString(ReasonExprMustBeConst);
      Rewriter_.insertTextAtEndOfLine(HERE, EndLoc, Reason);
      return true;
    }

    // Get the text of the original dereference expression (including the '*').
    const auto OriginalExprText =
        Rewriter_.getRewrittenText(OP->getSourceRange());
    if (!OriginalExprText.has_value()) {
      return true;
    }

    // Get the type of the pointer being dereferenced.
    clang::QualType PtrType = OP->getSubExpr()->getType();
    std::string ExprType = PtrType.getAsString();

    // Build the new expression with the runtime check.
    std::string NewExpr = std::string("*(NULL_CHECK( (" + ExprText.value() +
                                      "), " + ExprType + ") )");

    // Replace the original expression.
    Rewriter_.replaceText(HERE, OP->getSourceRange(), NewExpr);

    // Add comment with original code if configured.
    if (Config_.original_code_in_comment()) {
      std::string Comment =
          " // " + OriginalExprText.value() + " => " + NewExpr + "\n";

      Rewriter_.insertTextAtEndOfLine(HERE, OP->getBeginLoc(), Comment);
    }
  }

  return true; // true -> keep traversing the AST.
}
