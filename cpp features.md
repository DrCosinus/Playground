# C++ Features

## C++11

### alignas & alignof

> **Utilisation Actuelle** : Très peu Utilisé  
> **Directive Actuelle** : Quasiment jamais conseillé en revue  
> **Future Directive** : A utiliser dès que nécessaire

Le C++11 permet d'interroger et de controller l'alignement des variables grace aux mots clefs `alignof` et `alignas`.

### atomic operations

### auto

### C99 preprocessor

### constexpr

### decltype

### Defaulted and deleted functions

> **Utilisation Actuelle** : Utilisé  
> **Directive Actuelle** : Souvent conseillé en revue  
> **Future Directive** : A utiliser dès que possible

Le C++11 permet de demander ou d'empécher explicitement la génération des implémentation par défaut de ces fonctions membres spéciales (constructeur par défaut, constructeur par copie, opérateur d'assignation par copie, destructeur, ...).

```cpp
struct Foo
{
    Foo() = default; // Le constructeur par défaut est explicitement instancié
    Foo(int _value);
};

struct NonCopiable
{
    NonCopiable() = default;
    NonCopiable(const NonCopiable&) = delete;
    NonCopiable& operator=(const NonCopiable&) = delete;
}
```

Le spécificateur `= delete` peut être utilisé pour interdire l'appel à n'importe quel fonction. Cela peut être utilisé pour empécher l'appel de fonction membre avec certains types de paramètres (et se protejer des 'cast' implicites)

```cpp
struct NoInteger
{
    NoInteger(double) {}
    NoInteger(int) = delete;
};

struct OnlyDouble
{
    OnlyDouble(double) {}
    template<typename T> OnlyDouble(T) = delete;
};

void foo()
{
  //NoInteger p = 5; // error: conversion from 'int' to 'NoInteger' invokes a deleted function
  NoInteger q = 5.3;
  NoInteger r = 1.9f;
}

void bar()
{
  //OnlyDouble p = 5; // error: conversion from 'int' to 'NoInteger' invokes a deleted function
  OnlyDouble q = 5.3;
  //OnlyDouble r = 1.9f; // error: conversion from 'float' to 'OnlyDouble' invokes a deleted function
}
```

### Delegating constructors

> **Utilisation Actuelle** : Très peu utilisé  
> **Directive Actuelle** : Parfois conseillé en revue  
> **Future Directive** : A utiliser dès que possible

A partir du C++11 il est permis pour un constructeur de déléguer la création d'une instance à un autre constructeur.

```cpp
#include <iostream>

struct Foo
{
    Foo(int _value) : value_(_value) {}
    Foo() : Foo(42) {} // ce constructeur appelle le premier
    operator int() { return value_; } // implicit cast to int
private:
    int value_;
};

int main()
{
    Foo foo;
    std::cout << foo << std::endl;
    // output: 42
}
```

### Explicit conversion operators

### Extended friend declarations

### extern template

### Forward enum declarations

### inheriting constructors

> **Utilisation Actuelle** : Très peu utilisé  
> **Directive Actuelle** : Parfois conseillé en revue  
> **Future Directive** : A utiliser dès que possible

```cpp
#include <iostream>

struct Parent
{
    explicit Parent(int _value) : value_{_value} {}
    Parent() : value_(-1) {}
    Parent(int _value1, int _value2) : value_{_value1 + _value2} {}
    int get_value() const { return value_; }
    operator int() { return get_value(); } // implicit cast to int
private:
    int value_;
};

struct Child : Parent
{
    using Parent::Parent;
};

int main()
{
    Child child0, child1{14, 28}, child2{17};
    std::cout << child0 << ' ' << child1 << ' ' << child2 << std::endl;
    // output: -1 42 17
}

```

### initializer lists

### lambda expressions

### Local and unnamed types as template parameters

### long long

### Inline namespaces

### New character types

### Trailing function return types

### nullptr

### Unicode string literals

### Raw string literals

### User-defined literals

### Right angle brackets

### Rvalue references

### static_assert

> **Utilisation Actuelle** : Utilisé  
> **Directive Actuelle** : Conseillé en revue  
> **Future Directive** : A utiliser quand nécessaire

`static_assert` permet de tester des assertions au moment de la compilation.

```cpp
static_assert((MC_PI > 3.14) && (MC_PI < 3.15), "MC_PI is inaccurate!");

template<typename T>
struct Check
{
    static_assert(sizeof(int) <= sizeof(T), "T is not big enough!");
};

template<typename Integral>
Integral foo(Integral x, Integral y)
{
    static_assert(std::is_integral<Integral>::value, "foo() parameter must be an integral type.");
}
```

### Strongly-typed enum

### Template aliases

### Thread-local storage

C++11 introduit le mot-clef `thread_local` pour déclarer qu'une variable doit être stockée dans un zone mémoire appartenant au thread qui utilise la variable. Chaque thread embarque ainsi sa propre copie d'une telle variable.

### Unrestricted unions

### Type traits

### Variadic templates

### Range-for loop

### override and final

### attributes

Le C++11 fourni une syntaxe standard pour étendre le language. 

### ref qualifier

### non-static data member initializers

> **Utilisation Actuelle** : Peu utilisé  
> **Directive Actuelle** : Parfois conseillé en revue  
> **Future Directive** : A utiliser dès que possible

A partir du C++11, il est possible d'assigner une valeur par défaut aux variables membres non statique d'une classe.

```cpp
#include <iostream>

struct Foo
{
    Foo() {}
    explicit Foo(int _value) : value_(_value) {}
    operator int() { return value_; } // implicit cast to int
private:
    int value_ = 509;
};

int main()
{
    Foo foo;
    std::cout << foo << std::endl;
    // output: 509
}
```

Dans ce code, tous les constructeurs de la classe vont initialiser m_valeur à 5, si le constructeur ne remplace pas l'initialisation avec la sienne.

Par exemple, le constructeur vide ci-dessus va initialiser m_valeur selon la définition de la classe, mais le constructeur qui prend un int en paramètre initialisera m_valeur à ce paramètre.

Il est également possible d'utiliser d'autres attributs dans l'initialisation, et d'utiliser un constructeur ou l'utilisation uniforme au lieu de l'initialisation par assignation.

### dynamic initializaion and destruction with concurrency(magic statics)

### noexcept

### STL

#### tuples

#### hashtables (unordered_set, unordered_map, ...)

#### regex

#### smart pointers

`std::shared_ptr` `std::weak_ptr` `std::unique_ptr`

`std::auto_ptr` est déprécié.

#### Outils de nombres aléatoires

#### Wrapper de référence

#### std::begin, std::end

---

## C++14

### Tweaked wording for contextual conversions

### Binary literals

### decltype(auto), Return type deduction for normal functions

### Initialized/Generalized lambda captures (init-capture)

### Generic (polymorphic) lambda expressions

### Variable templates

### Extended constexpr

### Member initializers and aggregates (NSDMI)

### Clarifying memory allocation (avoiding/fusing allocations)

### Deprecated attribute

### Sized deallocation

### Single quote as digit separator

### std::result_of and SFINAE

### constexpr for `<complex>`

### constexpr for `<chrono>`

### constexpr for `<array>`

### constexpr for `<initializer_list>`, `<utility>` and `<tuple>`

### Improved std::integral_constant

### User-defined literals for `<chrono>` and `<string>`

### Null forward iterators

### std::quoted

### Heterogeneous associative lookup

### std::integer_sequence

### std::shared_timed_mutex

### std::exchange

### fixing constexpr member functions without const

### std::get\<T>()

### Dual-Range std::equal, std::is_permutation, std::mismatch

---

## C++17

### New auto rules for direct-list-initialization

### static_assert with no message

### typename in a template template parameter

### Removing trigraphs

### Nested namespace definition

### Attributes for namespaces and enumerators

### u8 character literals

### Allow constant evaluation for all non-type template arguments

### Fold Expressions

### Remove Deprecated Use of the register Keyword

### Remove Deprecated operator++(bool)

### Removing Deprecated Exception Specifications from C++17

### Make exception specifications part of the type system

### Aggregate initialization of classes with base classes

### Lambda capture of *this

### Using attribute namespaces without repetition

### Dynamic memory allocation for over-aligned data

### Unary fold expressions and empty parameter packs

### __has_include in preprocessor conditionals

### Template argument deduction for class templates

### Non-type template parameters with auto type

### Guaranteed copy elision

### New specification for inheriting constructors

### Direct-list-initialization of enumerations

### Stricter expression evaluation order

### constexpr lambda expressions

### Differing begin and end types in range-based for

### [[fallthrough]] attribute

### [[nodiscard]] attribute

### Pack expansions in using-declarations

### [[maybe_unused]] attribute

### Structured Bindings

### Hexadecimal floating-point literals

### Ignore unknown attributes

### constexpr if statements

### init-statements for if and switch

### Inline variables

### DR: Matching of template template-arguments excludes compatible templates

### /*No*/ Standardization of Parallelism TS

### std::uncaught_exceptions()

### /*No*/ Splicing Maps and Sets

### Improving std::pair and std::tuple

### std::shared_mutex (untimed)

### /*No*/ Elementary string conversions

### std::string_view

### /*No*/ std::filesystem