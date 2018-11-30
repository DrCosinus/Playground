# clang/llvm libTooling

Exploring clang tooling :

- [Part 0 - Building your code with clang](https://blogs.msdn.microsoft.com/vcblog/2018/09/18/exploring-clang-tooling-part-0-building-your-code-with-clang/)
- [Part 1 - Extending clang-tidy](https://blogs.msdn.microsoft.com/vcblog/2018/10/19/exploring-clang-tooling-part-1-extending-clang-tidy/)
- [Part 2 - Examining the clang AST with clang-query](https://blogs.msdn.microsoft.com/vcblog/2018/10/23/exploring-clang-tooling-part-2-examining-the-clang-ast-with-clang-query/)
- [Part 3 - Rewriting code with clang-tidy](https://blogs.msdn.microsoft.com/vcblog/2018/11/06/exploring-clang-tooling-part-3-rewriting-code-with-clang-tidy/)

[AST Matcher reference](http://clang.llvm.org/docs/LibASTMatchersReference.html)

[Clang doxygen](https://clang.llvm.org/doxygen/classclang_1_1FunctionDecl.html)

clang-check file.cpp --

clang-query file.cpp --

## Queries

Tests:

```plaintext
set output dump
m cxxRecordDecl()
m cxxRecordDecl(isExpansionInMainFile())
m cxxRecordDecl(isExpansionInMainFile(),unless(isImplicit()))
m functionDecl()
m cxxMethodDecl(isVirtualAsWritten())
```

Complete classes and structures:

```plaintext
m cxxRecordDecl(isExpansionInMainFile(),unless(isImplicit()),hasDefinition())
```

Uncomplete classes and structures:

```plaintext
m cxxRecordDecl(isExpansionInMainFile(),unless(isImplicit()),unless(hasDefinition()))
```

Call memberfunction on type

```plaintext
m cxxMemberCallExpr(on(hasType(cxxRecordDecl(hasName("Base")))))

m cxxMemberCallExpr(on(hasType(pointsTo(cxxRecordDecl(hasName("Base"))))))

m cxxMemberCallExpr(on(anyOf( hasType(pointsTo(cxxRecordDecl(hasName("Base")))), hasType(cxxRecordDecl(hasName("Base"))) )))

```

Declare object of type

```plaintext
VarDecl 0x19c6606f128 <col:5, col:10> col:10 b 'Base' callinit
```

Calling new

```plaintext
`-CXXNewExpr 0x296ff25c4b8 <col:15, col:24> 'Base *' Function 0x296ff25bee8 'operator new' 'void *(unsigned long long)'
  `-CXXConstructExpr 0x296ff25c488 <col:19, col:24> 'Base' 'void () noexcept' zeroing
```

Declaration with copy

```plaintext
m varDecl(has(cxxConstructExpr()))
m varDecl(hasDescendant(cxxConstructorDecl() ))
m varDecl(hasDescendant(cxxConstructorDecl(isCopyConstructor()) ))
m varDecl(has(cxxConstructExpr()))
```

## Includes

not needed definition:

- declaration of overriden member function parameter types (parent class has already fo the job)

need uncomplete definition:

- member pointer to type
- member reference to type
- function parameter as pointer to type
- function parameter as reference to type

need complete definition:

- inherit from
- operator dot
- operator arrow

- variable declaration of type (not pointer, not reference)
- operator new

- overriden member functions implies at least that the type of their parameters and return value type were forward declared for the parent class.

- alias (using and typedef)

- definition of template class specializations

- definition of macros

## To do

fix backslash, fix relative, fix angle brackets vs double quotes, precompiledheader in cpp

declaration without definition detection + removal

dead code detection + removal

renaming convention checker + fixer

function reordering

implicit convertion detection

new A or new A{} or new A()

overriden methods do not call its parent

member method does not use other members -> consider free function or static member function

detect copy due to auto instead of auto&

a member function which does not use member variable or function could be static

a non const member function which does not use modify member variable or call only member functions could be const

cmake .. -G "Visual Studio 15 2017" -DCMAKE_GENERATOR_PLATFORM=x64 -Thost=x64 -DLLVM_INCLUDE_TESTS=OFF -DLLVM_BUILD_TOOLS=OFF -DLLVM_INCLUDE_UTILS=OFF -DLLVM_TARGETS_TO_BUILD="" -DCLANG_ENABLE_STATIC_ANALYZER=OFF -DCLANG_ENABLE_ARCMT=OFF

Updating misc\CMakeLists.txt...
Creating misc\MyFirstCheckCheck.h...
Creating misc\MyFirstCheckCheck.cpp...
Updating misc\MiscTidyModule.cpp...
Updating ..\docs\ReleaseNotes.rst...
Creating ..\test\clang-tidy\misc-my-first-check.cpp...
Creating ..\docs\clang-tidy\checks\misc-my-first-check.rst...
Updating ..\docs\clang-tidy\checks\list.rst...