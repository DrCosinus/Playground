# Blueprints

Blueprints are json files.

## Extension

The extension .blueprint.json is not mandatory, for now it is recommended.

## Project

### Paths

Paths are relative to blueprint file location.

### Build condition

A project must be build if anything in its files (sources, headers...) or any files in one of its dependencies (recursively)

### Sources and Headers

Sources and headers can use jokers

#### Jokers

trailing ... at the end of a path means everything in the folder recursively
trailing * at the end of a path means all files in the folder (non recursively)

## Dependencies

Dependencies must be any projects declared before the project which depends on them.

Dependencies means that:

- the include_paths of the projects on which the current project depends are append to the include_paths of the current project
- the lib_paths of the projects on which the current project depends are append to the lib_paths of the current project

---

In file `wit.blueprint` in the folder `wit`:

```json
{
  "project":
  {
    "name": "wit",
    "headers": "nonintegral_enum.hpp"
  }
}
```

In file `tdd.blueprint` in the folder `tdd`:

```json
{
  "project":
  {
    "name": "tdd",
    "headers": "simple_tests.hpp"
  }
}
```

In file `catch2.blueprint` in the folder `extern_libs`:

```json
{
  "project":
  {
    "name": "catch2",
    "headers": "catch.hpp"
  }
}
```

In file `nonintegral_enum_tests.blueprint`:

```json
{
  "import" : [ "wit/wit.blueprint", "tdd/tdd.blueprint" ],
  "project":
  {
    "name": "nonintegral_enum_tests",
    "dependencies": [ "tdd", "wit" ],
    "sources": "enum.cpp",
    "output": "bin/$(project_name)_$(compiler_name)_$(optimization).exe"
  }
}
```

In file `grammar.blueprint`:

```json
{
  "import": "tdd/tdd.blueprint",
  "project":
  {
    "name":  "grammar",
    "dependencies": "tdd",
    "sources": "grammar.cpp",
    "output": "bin/$(project_name)_$(compiler_name)_$(optimization).exe",
    "debug":
    {
      "warning_level": "max"
    }
  }
}
```

In file `strong_type.blueprint`:

```json
{
  "import": "extern_libs/catch2.blueprint",
  "project":
  {
    "name": "strong_type_tests",
    "dependencies": "catch2",
    "headers": "strong_type.h",
    "sources": "strong_type_tests.cpp",
    "output": "bin/$(project_name)_$(compiler_name)_$(optimization).exe"
  }
}
```