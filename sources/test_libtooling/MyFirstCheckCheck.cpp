//===--- MyFirstCheckCheck.cpp - clang-tidy -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MyFirstCheckCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"

#pragma optimize("", off)
using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace misc {

namespace {
struct IncludePPCallbacks : PPCallbacks {
  IncludePPCallbacks(ClangTidyCheck &Check, SourceManager &SM)
      : Check(Check), SM(SM) {}

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange, const FileEntry *File,
                          StringRef SearchPath, StringRef RelativePath,
                          const Module *Imported,
                          SrcMgr::CharacteristicKind FileType) override;

private:
  struct IncludeDirective {
    SourceLocation Loc;    ///< '#' location in the include directive
    CharSourceRange Range; ///< SourceRange for the file name
    std::string Filename;  ///< Filename as a string
    bool IsAngled;         ///< true if this was an include with angle brackets
    bool IsMainModule;     ///< true if this was the first include in a file
  };
  bool LookForMainModule = true;

  ClangTidyCheck &Check;
  SourceManager &SM;
};
} // namespace

void IncludePPCallbacks::InclusionDirective(
    SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName,
    bool IsAngled, CharSourceRange FilenameRange, const FileEntry *File,
    StringRef SearchPath, StringRef RelativePath, const Module *Imported,
    SrcMgr::CharacteristicKind FileType) {
  // We recognize the first include as a special main module header and want
  // to leave it in the top position.
  if (SM.getFileID(HashLoc) == SM.getMainFileID()) {
    IncludeDirective ID = {HashLoc, FilenameRange, FileName, IsAngled, false};

    auto loc = HashLoc.printToString(SM);
    if (LookForMainModule && !IsAngled) {
      ID.IsMainModule = true;
      LookForMainModule = false;
    }
  }
  // Bucket the include directives by the id of the file they were declared in.
  // IncludeDirectives[SM.getFileID(HashLoc)].push_back(std::move(ID));
}

void MyFirstCheckCheck::registerPPCallbacks(CompilerInstance &Compiler) {
  Compiler.getPreprocessor().addPPCallbacks(
      ::llvm::make_unique<IncludePPCallbacks>(*this,
                                              Compiler.getSourceManager()));
}
void MyFirstCheckCheck::registerMatchers(MatchFinder *Finder) {
  // FIXME: Add matchers.
  Finder->addMatcher(
      cxxRecordDecl(
          isExpansionInMainFile(),
          isDerivedFrom(
              cxxRecordDecl(unless(isExpansionInMainFile())).bind("base")))
          .bind("derived"),
      this);
}
void MyFirstCheckCheck::check(const MatchFinder::MatchResult &Result) {
  // FIXME: Add callback implementation.
  const auto DerivedDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("derived");
  const auto BaseDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("base");
  auto name = DerivedDecl->getNameAsString();
  auto base_name = BaseDecl->getNameAsString();
  // if (MatchedDecl->getName().startswith("awesome_"))
  //  return;
  diag(DerivedDecl->getLocation(),
       "struct/class %0 need the full definition of struct/class %1",
       DiagnosticIDs::Remark)
      << DerivedDecl << BaseDecl;
  //<< FixItHint::CreateInsertion(DerivedDecl->getLocation(), "awesome_");
}

} // namespace misc
} // namespace tidy
} // namespace clang
