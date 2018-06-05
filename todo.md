# TODO

## unmutable string view

- constructible from const char*, literal string, std::string, custom strings, ... (expendability)
- cheap to convert to const char*, std::string, ...

## array view

## determinism helpers

## query selector

auto AllProps = Entities.Select([](auto& s){ return s.IsProp() });
