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

#include <iostream>

// see IncludeOrderCheck

#pragma optimize("", off)
using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace misc {

namespace {
struct IncludePPCallbacks : PPCallbacks {
  IncludePPCallbacks(MyFirstCheckCheck &Check, SourceManager &SM)
      : Check(Check), SM(SM) {}

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange, const FileEntry *File,
                          StringRef SearchPath, StringRef RelativePath,
                          const Module *Imported,
                          SrcMgr::CharacteristicKind FileType) override;
  void EndOfMainFile() override;

private:
  struct IncludeDirective {
    SourceLocation Loc;    ///< '#' location in the include directive
    CharSourceRange Range; ///< SourceRange for the file name
    std::string Filename;  ///< Filename as a string
    bool IsAngled;         ///< true if this was an include with angle brackets
    bool IsMainModule;     ///< true if this was the first include in a file
  };
  bool LookForMainModule = true;

  MyFirstCheckCheck &Check;
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
  SM = &Compiler.getSourceManager();
  Compiler.getPreprocessor().addPPCallbacks(
      ::llvm::make_unique<IncludePPCallbacks>(*this,
                                              Compiler.getSourceManager()));
}
void MyFirstCheckCheck::registerMatchers(MatchFinder *Finder) {
  // forward declaration of classes and structures in "main" file
  Finder->addMatcher(
      cxxRecordDecl(/*isExpansionInMainFile(),*/
                    unless(hasDefinition()), unless(isImplicit()))
          .bind("forwardDeclaredRecord"),
      this);
  // complete declaration of classes and structures in "main" file
  Finder->addMatcher(cxxRecordDecl(/*isExpansionInMainFile(),*/ hasDefinition(),
                                   unless(isImplicit()))
                         .bind("fullyDeclaredRecord"),
                     this);

  // class/struct inherits from type => complete definition of type needed
  Finder->addMatcher(
      cxxRecordDecl(isExpansionInMainFile(),
                    isDerivedFrom(cxxRecordDecl(unless(isExpansionInMainFile()))
                                      .bind("baseRecord")))
          .bind("inheritingRecord"),
      this);

  // member function calls (dot or arrow operator) on instance of type =>
  // complete definition of type needed
  Finder->addMatcher(cxxMemberCallExpr(isExpansionInMainFile(),
                                       on(hasType(cxxRecordDecl().bind(
                                           "callMemberFunctionOnType"))))
                         .bind("memberCall"),
                     this);

  // static member function calls ??

  // declaration of object of type... (unless pointer or reference)

  // operator new call on type
  /*
  `-CXXNewExpr 0x296ff25c4b8 <col:15, col:24> 'Base *' Function 0x296ff25bee8
  'operator new' 'void *(unsigned long long)'
  `-CXXConstructExpr 0x296ff25c488 <col:19, col:24> 'Base' 'void () noexcept'
  zeroing
  */
}

void MyFirstCheckCheck::check(const MatchFinder::MatchResult &Result) {

  if (const auto forwardDeclaredRecordDecl =
          Result.Nodes.getNodeAs<CXXRecordDecl>("forwardDeclaredRecord")) {
    forwardDeclaredRecords[forwardDeclaredRecordDecl->getName()].push_back(
        {RecordDeclarationKind::IsIncomplete,
         SM->isInMainFile(forwardDeclaredRecordDecl->getLocation())
             ? RecordDeclarationRegion::IsInMain
             : RecordDeclarationRegion::IsInElsewhere,
         forwardDeclaredRecordDecl->getLocation()});
    // need to avoid to push same location multiple times
  }
  if (const auto fullyDeclaredRecordDecl =
          Result.Nodes.getNodeAs<CXXRecordDecl>("fullyDeclaredRecord")) {
    forwardDeclaredRecords[fullyDeclaredRecordDecl->getName()].push_back(
        {RecordDeclarationKind::IsComplete,
         SM->isInMainFile(fullyDeclaredRecordDecl->getLocation())
             ? RecordDeclarationRegion::IsInMain
             : RecordDeclarationRegion::IsInElsewhere,
         fullyDeclaredRecordDecl->getLocation()});
  }
  if (const auto BaseDecl =
          Result.Nodes.getNodeAs<CXXRecordDecl>("baseRecord")) {
    const auto DerivedDecl =
        Result.Nodes.getNodeAs<CXXRecordDecl>("inheritingRecord");

    needs[BaseDecl->getNameAsString()].push_back(
        DerivedDecl->getBeginLoc().printToString(*SM));
  }
  if (const auto CompletelyDefinedRecord =
          Result.Nodes.getNodeAs<CXXRecordDecl>("callMemberFunctionOnType")) {
    const auto &memberCallExpr =
        Result.Nodes.getNodeAs<CXXMemberCallExpr>("memberCall");
    needs[CompletelyDefinedRecord->getNameAsString()].push_back(
        memberCallExpr->getBeginLoc().printToString(*SM));
  }

  // diag(DerivedDecl->getLocation(),
  //     "struct/class %0 need the full definition of struct/class %1",
  //     DiagnosticIDs::Remark)
  //    << DerivedDecl << BaseDecl;
  //<< FixItHint::CreateInsertion(DerivedDecl->getLocation(), "awesome_");
}

void IncludePPCallbacks::EndOfMainFile() {

  std::cout << "--==####==--" << std::endl;

  for (auto &p : Check.needs) {
    auto &base = p.first;
    std::cout << "Need full definition of Record '" << base
              << "' due to :" << std::endl;
    auto &usages = p.second;
    for (auto &u : usages) {
      std::cout << "-\t'" << u << "'" << std::endl;
    }
  }

  std::cout << "Forward declared strutures and classes:" << std::endl;
  for (auto &pair_RecordName_Locations : Check.forwardDeclaredRecords) {
    auto &recordName = pair_RecordName_Locations.first;
    auto &locations = pair_RecordName_Locations.second;

    auto inMainCount =
        std::count_if(std::begin(locations), std::end(locations),
                      [this](const RecordDeclarationInfo &k) {
                        return k.region ==
                               RecordDeclarationRegion::
                                   IsInMain /*SM.isInMainFile(k.location)*/;
                      });
    auto CompleteDeclarationCount =
        std::count_if(std::begin(locations), std::end(locations),
                      [this](const RecordDeclarationInfo &k) {
                        return k.kind == RecordDeclarationKind::IsComplete;
                      });

    std::cout << "-\t" << recordName.str() << " ( " << inMainCount << "/"
              << locations.size() << " ) Complete:" << CompleteDeclarationCount
              << std::endl;
  }
}

} // namespace misc
} // namespace tidy
} // namespace clang
