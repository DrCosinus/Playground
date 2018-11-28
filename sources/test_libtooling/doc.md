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

fix backslash, fix relative, fix angle brackets vs double quotes, precompiledheader in cpp