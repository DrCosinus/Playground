# Grammar

Experiment of compile time expression to analyse strings.

[Regular Expressions](https://en.wikipedia.org/wiki/Regular_expression)

## TERMINALS

### One character among a list

> grammar::char_among

```cpp
using blank = char_among<' ', '\t'>;
using space = char_among<' ', '\t', '\r', '\n', '\v', '\f'>;
```

### One character in a range

> grammar::char_in_range

== TO BE IMPLEMENTED ==

Limits are included.

```cpp
using digit = char_in_range<'0', '9'>;
using lower = char_in_range<'a', 'z'>;
using upper = char_in_range<'A', 'Z'>;
```

### Any character

> grammar::any_char

## COMPOSITOR

### Any 'predicates' of a list

> grammar::any_of

```cpp
using alpha = any_of<lower, upper>;
using alnum = any_of<alpha, digit>;
using word = any_of<alnum, char_among<'_'>>;
using xdigit = any_of<digit, char_in_range<'a', 'f'>, char_in_range<'A', 'F'>>;
```

### Invert the 'predicate'

> grammar::is_not

```cpp
using not_word = is_not<word>;
```

## At least a number of characters matching the predicate
using grammar::at_least;

## A sequence of characters matching the predicates in order
using grammar::sequence;

## At most one character matching the predicate
using grammar::optional;

## CHARACTER CLASSES

- Blanks: space and tabs
- Spaces
- Digits
- Lower
- Upper
- Xdigits
- word
- Alnum
- not_word

## ALGORITHMS

using grammar::search;
using grammar::match;

## TODO

    // some classical terminals to be considered
    // - digit
    // - alpha
    // - alphanum
    // - start of line
    // - end of line
    // - start of file
    // - end of file

    // boundary aka \b

[Back to README](../../README.md)