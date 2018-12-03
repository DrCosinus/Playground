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

namespace clang {
namespace tidy {
namespace misc {

enum class Completeness {
  Complete,
  Incomplete,
};

enum class DeclarationScope {
  InMain,
  InHeader, // only if main is a cpp with same name
  Elsewhere
};

struct DeclarationInfo {
  explicit DeclarationInfo(std::string name) : name{name} {}
  std::vector<SourceLocation> completeDeclarations;
  std::vector<SourceLocation> incompleteDeclarations;
  std::string name;
};

struct DeclarationNeed {
  std::string name;
  Completeness kind;
  SourceLocation location;
};

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

  std::vector<DeclarationNeed> declarationNeeds;

  std::vector<DeclarationInfo> declarationInfos;

private:
  void addDeclaration(const CXXRecordDecl &cxxRecordDecl);
  void addNeed(const NamedDecl &need, Completeness completeness,
               SourceLocation where);

  SourceManager *SM = nullptr;
};

} // namespace misc
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_MYFIRSTCHECKCHECK_H
