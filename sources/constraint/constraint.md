# Constraint Checker

Some experimentation of tool classes to help coders to check some values (function arguments, function returned value) during production.

## Goal

We create a templated type `Constrained<T>` which encapsulate a type `T`.

A `Constrained<T>` is implicitly constructible from a `T`.

A `Constrained<T>` is implicitly convertible to a `T`.

A `Constrained<T>` is implicitly assignble from a `T`.

When a `Constrained<T>` is constructed from a `T` some requirements on its value are checked.

For flexibility, requirements are provided thank to a function object type `REQUIREMENT_CHECKER` provided as template type argument.

Failure are handled by `REQUIREMENT_CHECKER`.

The `Constrained<T>` type must checker the same unit tests than the `T` type.

Optionally, requirements can be checked when `Constrained<T>` is modified (ConstructionOnly vs AnyModification).

### Functions arguments check

```cpp
struct FailureHandler
{
    void operator()(std::string _message) const
    {
        std::cout << _message << std::endl;
        exit(0);
    }
}

float square_root(Constrained<float, GreaterOrEquakThan<0, FailureHandler<WorkdAround>, ConstructionOnly>> _value)
{
    return std::sqrt(_value);
}
```

### Predefined Requirements

#### Combining multiple requirements

- `Or<Requirements...>`
- `And<Requirements...>`
- `Not<Requirement>`

#### Arithmetic type

- `IsEqualTo<Value>`
- `IsNotEqualTo<Value>`
- `GreaterThan<Threshold>`
- `GreaterThan<Threshold>`
- `GreaterOrEqualThan<Threshold>`
- `LessThan<Threshold>`
- `LessOrEqualThan<Threshold>`
- `IsInsideRange<FirstBound, SecondBound>`
- `IsOutsideRange<FirstBound, SecondBound>`

##### Floating-point type

- `IsNearlyEqualTo<Value>`

#### Pointer type

- `IsNotNull`

### User defined requirements

- `VectorIsNormal`
- `VectorNotNull`

---

## Output folder

> Folder: `bin/{host}/{compiler}/{optimization}/executable.exe`

|              |  possible values   |
|--------------|:------------------:|
| host         | windows / linux    |
| compiler     | clang / gcc / msvc |
| optimization | debug / release    |

[Back to README](../../README.md)