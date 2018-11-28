//===--- MyFirstCheckCheck.h - clang-tidy -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_MYFIRSTCHECKCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_MYFIRSTCHECKCHECK_H

#include "../ClangTidy.h"
#include "../utils/IncludeInserter.h"

namespace clang {
namespace tidy {
namespace misc {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/misc-my-first-check.html
class MyFirstCheckCheck : public ClangTidyCheck {
public:
  MyFirstCheckCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}
  void registerPPCallbacks(CompilerInstance &Compiler) override;
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  std::unique_ptr<clang::tidy::utils::IncludeInserter> IncludeInserter;
  const utils::IncludeSorter::IncludeStyle IncludeStyle =
      utils::IncludeSorter::IncludeStyle::IS_LLVM;
};

} // namespace misc
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_MYFIRSTCHECKCHECK_H
