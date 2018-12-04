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
      : Check{Check}, SM{SM} {}

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
  };
  // bool LookForMainModule = true;

  MyFirstCheckCheck &Check;
  SourceManager &SM;
  std::vector<IncludeDirective> includeDirectives;
};
} // namespace

void IncludePPCallbacks::InclusionDirective(
    SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName,
    bool IsAngled, CharSourceRange FilenameRange, const FileEntry *File,
    StringRef SearchPath, StringRef RelativePath, const Module *Imported,
    SrcMgr::CharacteristicKind FileType) {

  // we store only the file included directly in main file
  if (SM.isInMainFile(HashLoc)) {
    IncludeDirective ID = {HashLoc, FilenameRange, FileName};
    includeDirectives.push_back(std::move(ID));
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

  // all declarations of classes, structures (and unions)
  // in main file or in other files
  // with or without definition
  // avoid specializations
  Finder->addMatcher(cxxRecordDecl(unless(isImplicit()),
                                   unless(isExplicitTemplateSpecialization()))
                         .bind("declaredRecord"),
                     this);

  // ** class/struct inherits from type **
  //=> complete definition of type needed
  Finder->addMatcher(
      cxxRecordDecl(isExpansionInMainFile(),
                    isDerivedFrom(cxxRecordDecl(unless(isExpansionInMainFile()))
                                      .bind("baseRecord")))
          .bind("inheritingRecord"),
      this);

  // ** member function calls (dot or arrow operator) on instance of type **
  // => complete definition of type needed
  Finder->addMatcher(cxxMemberCallExpr(isExpansionInMainFile(),
                                       on(hasType(cxxRecordDecl().bind(
                                           "callMemberFunctionOnType"))))
                         .bind("memberCall"),
                     this);

  // static member function calls ??
  Finder->addMatcher(
      callExpr(isExpansionInMainFile(),
               hasDeclaration(cxxMethodDecl(isStaticStorageClass())
                                  .bind("callStaticMemberFunctionOnType")))
          .bind("staticMemberCall"),
      this);

  // function pointer or reference parameter
  Finder->addMatcher(
      functionDecl(isExpansionInMainFile(), unless(isImplicit()),
                   hasDescendant(parmVarDecl(anyOf(hasType(referenceType()),
                                                   hasType(isAnyPointer())))
                                     .bind("ptrOrRefParm")))
          .bind("functionOrMethod"),
      this);

  // declaration of object of type... (unless pointer or reference)

  // operator new call on type
  /*
  `-CXXNewExpr 0x296ff25c4b8 <col:15, col:24> 'Base *' Function 0x296ff25bee8
  'operator new' 'void *(unsigned long long)'
  `-CXXConstructExpr 0x296ff25c488 <col:19, col:24> 'Base' 'void () noexcept'
  zeroing
  */
}

// void RecordDeclarationInfo::insert(const CXXRecordDecl &cxxRecordDecl) {
//  declarations.push_back(&cxxRecordDecl);
//}
//: location{cxxRecordDecl->getLocation()},
//  kind{cxxRecordDecl->hasDefinition()
//           ? Completeness::IsComplete
//           : Completeness::IsIncomplete},
//  region{SM.isInMainFile(location)
//             ? RecordDeclarationRegion::IsInMain
//             : RecordDeclarationRegion::IsInElsewhere} {}

void MyFirstCheckCheck::addDeclaration(const CXXRecordDecl &cxxRecordDecl) {
  auto name = cxxRecordDecl.getQualifiedNameAsString();
  auto isComplete = cxxRecordDecl.hasDefinition();
  auto location = cxxRecordDecl.getLocation();

  auto it =
      std::find_if(std::begin(declarationInfos), std::end(declarationInfos),
                   [&name](const auto &x) { return x.name == name; });
  if (it == std::end(declarationInfos)) {
    declarationInfos.push_back(DeclarationInfo{name});
    it = declarationInfos.end() - 1;
  }
  auto &infoList =
      isComplete ? it->completeDeclarations : it->incompleteDeclarations;
  if (std::end(infoList) ==
      std::find_if(std::begin(infoList), std::end(infoList),
                   [&location](const SourceLocation &_location) {
                     return location == _location;
                   })) {
    infoList.push_back(location);
  }

  // need to avoid to push same location multiple times
}

void MyFirstCheckCheck::addNeed(const NamedDecl &need,
                                Completeness completeness,
                                SourceLocation where) {
  declarationNeeds.push_back(
      {need.getQualifiedNameAsString(), completeness, where});
}

void MyFirstCheckCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto declaredRecordDecl =
          Result.Nodes.getNodeAs<CXXRecordDecl>("declaredRecord")) {

    addDeclaration(*declaredRecordDecl);
    return;
  }

  if (const auto BaseDecl =
          Result.Nodes.getNodeAs<CXXRecordDecl>("baseRecord")) {
    const auto DerivedDecl =
        Result.Nodes.getNodeAs<CXXRecordDecl>("inheritingRecord");

    addNeed(*BaseDecl, Completeness::Complete, DerivedDecl->getLocation());
    return;
  }

  if (const auto CompletelyDefinedRecord =
          Result.Nodes.getNodeAs<CXXRecordDecl>("callMemberFunctionOnType")) {
    const auto memberCallExpr =
        Result.Nodes.getNodeAs<CXXMemberCallExpr>("memberCall");
    addNeed(*CompletelyDefinedRecord, Completeness::Complete,
            memberCallExpr->getBeginLoc());
    return;
  }

  if (const auto parmVarDecl =
          Result.Nodes.getNodeAs<ParmVarDecl>("ptrOrRefParm")) {
    const auto memberCallExpr =
        Result.Nodes.getNodeAs<CXXMemberCallExpr>("functionOrMethod");

    // FIXME: Need to retrieve pointed/referenced type
    auto qualType = parmVarDecl->getType();
    auto isCanon = qualType.isCanonical();
    std::string def;
    auto typePtr = qualType.getTypePtrOrNull();
    
    auto qt = qualType;
    std::string tn;
    while (qt->isPointerType())
        {
      qt = qt->getPointeeType();
        }
    if (qt->isRecordType())
    {
          auto rd = qt->getAsCXXRecordDecl();
      tn = rd->getQualifiedNameAsString();
    }


    auto qp = qualType->getPointeeCXXRecordDecl();
    if (qp) {
      auto h = qp->getQualifiedNameAsString();
      auto é = 3;
    }
    if (typePtr) {
      auto k = typePtr->getPointeeCXXRecordDecl(); //

      if (k) {
        def = k->getQualifiedNameAsString();
      }
    } else {
      // def = qualType.getAsC
      //             .getTypePtr()
      //             ->getAsCXXRecordDecl()
      //             ->getQualifiedNameAsString();
    }
    //->getAsCXXRecordDecl();

    auto t = parmVarDecl->getDeclName().getAsString();
    auto _ = 0;
  }

  // diag(DerivedDecl->getLocation(),
  //     "struct/class %0 need the full definition of struct/class %1",
  //     DiagnosticIDs::Remark)
  //    << DerivedDecl << BaseDecl;
  //<< FixItHint::CreateInsertion(DerivedDecl->getLocation(), "awesome_");
}

void IncludePPCallbacks::EndOfMainFile() {
  std::cout << "--==####==--" << std::endl;

  for (auto &p : Check.declarationNeeds) {
    bool isComplete = p.kind == Completeness::Complete;
    std::cout << (isComplete ? "(*) " : "( ) ") << p.name << std::endl
              << '\t' << p.location.printToString(SM) << std::endl;
    auto itDeclInfo = std::find_if(
        std::begin(Check.declarationInfos), std::end(Check.declarationInfos),
        [&p](const auto &d) { return p.name == d.name; });
    if (itDeclInfo == std::end(Check.declarationInfos)) {
      std::cout << "NOT DECLARED!!" << std::endl;
      continue;
    }
    // std::cout << "Declaration found." << std::endl;
    auto &declarations = isComplete ? itDeclInfo->completeDeclarations
                                    : itDeclInfo->incompleteDeclarations;
    bool success = false;
    std::find_if(std::begin(declarations), std::end(declarations),
                 [isComplete, &success, &SM = SM](const SourceLocation &loc) {
                   if (isComplete) {
                     std::cout << "Need incude of " << loc.printToString(SM)
                               << std::endl;
                   }
                   return false;
                 });
  }
  std::cout << std::endl;
  /*
  std::cout << "Forward declared strutures and classes:" << std::endl;
  for (auto &pair_RecordName_Locations : Check.recordDeclareInfos) {
    auto &recordName = pair_RecordName_Locations.first;
    auto &locations = pair_RecordName_Locations.second;

    auto inMainCount =
        std::count_if(std::begin(locations), std::end(locations),
                      [this](const RecordDeclarationInfo &k) {
                        return k.region ==
                               RecordDeclarationRegion::
                                   IsInMain;
                      });
    auto CompleteDeclarationCount = std::count_if(
        std::begin(locations), std::end(locations),
        [this](const RecordDeclarationInfo &k) {
          return k.kind == Completeness::
                               IsComplete
              ;
        });
    if (CompleteDeclarationCount > 1 || inMainCount > 0) {
      std::cout << "-" << recordName << " ( " << inMainCount << "/"
                << locations.size()
                << " ) Complete:" << CompleteDeclarationCount << std::endl;
      for (const auto &location : locations) {
        std::cout << (location.kind == Completeness::IsComplete
                          ? "Complete"
                          : "Forward")
                  << " - " << location.location.printToString(SM) <<
  std::endl;
      }
    }
  }
  std::cout << std::endl;*/

  std::cout << "Includes in main file:" << std::endl;
  for (const auto &includeDirective : includeDirectives) {
    std::cout << "- " << includeDirective.Filename << " "
              << includeDirective.Loc.printToString(SM) << std::endl;
  }
  std::cout << std::endl;
  /*
  std::cout << "Forward declaration in main file:" << std::endl;
  for (const auto &x : Check.declarationInfos) {
    auto completeCount = x.completeDeclarations.size();
    auto incompleteCount = x.incompleteDeclarations.size();

    std::cout << "- " << x.name << " ( " << completeCount << "/"
              << completeCount + incompleteCount << " )" << std::endl;
    if (completeCount > 1) {
      for (const auto &d : x.completeDeclarations) {
        std::cout << "\t- " << d.printToString(SM) << "(*)"
                  << (SM.isInMainFile(d) ? "inMain" : "") << std::endl;
      }
      for (const auto &d : x.incompleteDeclarations) {
        std::cout << "\t- " << d.printToString(SM) << "( )"
                  << (SM.isInMainFile(d) ? "inMain" : "") << std::endl;
      }
    }
    for (const auto &d : x.completeDeclarations) {
      if (SM.isInMainFile(d)) {
        std::cout << "\t# " << d.printToString(SM) << "(*)"
                  << "inMain" << std::endl;
      }
    }
    for (const auto &d : x.incompleteDeclarations) {
      if (SM.isInMainFile(d)) {
        std::cout << "\t# " << d.printToString(SM) << "( )"
                  << "inMain" << std::endl;
      }
    }
  }*/
}

} // namespace misc
} // namespace tidy
} // namespace clang
