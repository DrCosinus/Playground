# To Do

## Check that the return value of a function is used (copied or moved)

## Check that a function is called

## Ensure that a (non const) reference function parameter is set

eventually consider to provide a default value (fallback value)

## experimental::constrained of T should pass the same unit tests than T

## others

```cpp
Constrained<int, And<NotEqual<int, 42>, NotEqual<int, 17>>, FailureHandler, AllModificationsPolicy>>;
```

_FailureHandler_ et _CheckPolicy_ doivent être sorti des paramètres template du `Checker` et doivent plutôt passer dans les paramètres template du `Constrained`.

### Hmmm

The tests should be run with "runtime" values, and an argument could be constrained according to some other arguments

For example:

```cpp
template<typename VALUE_TYPE, typename FACTOR_TYPE>
float Lerp(
    VALUE_TYPE _value_zero,
    VALUE_TYPE _value_one,
    FACTOR_TYPE _alpha
    );
```

In the "Lerp" function, we could assert that:

- `_value_zero` is less than `_value_one` (this is not necessarily wise).
- `_alpha` is between 0 and 1

```cpp
template<typename VALUE_TYPE, typename FACTOR_TYPE>
float Lerp(
    VALUE_TYPE _value_zero,
    VALUE_TYPE _value_one,
    Constrained<FACTOR_TYPE, IsInsideRange<FACTOR_TYPE, 0, 1>> _alpha
    );
```