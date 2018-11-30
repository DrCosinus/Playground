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

enum class RecordDeclarationKind {
  IsComplete,
  IsIncomplete,
};

enum class RecordDeclarationRegion {
  IsInMain,
  IsInHeader, // only if main is a cpp with same name
  IsInElsewhere
};

struct RecordDeclarationInfo {
  RecordDeclarationInfo(const SourceManager &SM,
                        const CXXRecordDecl *cxxRecordDecl);

  bool operator==(const RecordDeclarationInfo &other) const {
    return kind == other.kind && location == other.location;
  }
  SourceLocation location;
  RecordDeclarationKind kind;
  RecordDeclarationRegion region;
};

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

  std::map<std::string, std::vector<std::string>> needs;

  std::map</*llvm::StringRef*/ std::string, std::vector<RecordDeclarationInfo>>
      recordDeclareInfos;

private:
  void addRecordDeclaration(const CXXRecordDecl *cxxRecordDecl);

  SourceManager *SM = nullptr;
};

} // namespace misc
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_MYFIRSTCHECKCHECK_H
