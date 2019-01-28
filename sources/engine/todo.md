# TODO

## Features

- [ ] engine sub project should be extract from the playground repository to its own repository
- [ ] engine should be split in two: engine.exe and game.dll
  game.dll should be "hot" compilable
- [ ] implementing tileset (square, rectangle, hexagon, custom...)
- [ ] implementing something like libfmt...
- [ ] implementing something like moustache (debugging logs history, analysis facilities)
- [ ] implementing mathlib (vector/matrix 2d/3d)
- [ ] consider to excavate synapse:
  - [ ] split renderer (to be shared with other tools) & the message system
    renderer should be rely on an abstraction of DirectX/OpenGL/GDI or (why not?) the renderer of the engine
- [ ] profiling/benching facilities
- [ ] memory management (virtual memory)
- [ ] sound
  - [ ] basic wave forms: square, triangle, sawtooth, sine, noise, ...
- [ ] loading bitmaps

## References

- [C++](https://cppreference.com)

## Guidelines

- [x] use of [C++17](https://isocpp.org/), support [MSVC](http://visualstudio.com) (from version 19.16, maybe previous), [Clang](http://llvm.org/) (from version 7.0?)
- [x] using clang-format

### Documentation

| Name                                                                                           |                           | Current Version |
| ---------------------------------------------------------------------------------------------- | ------------------------- | --------------- |
| [PlantUML](https://marketplace.visualstudio.com/items?itemName=jebbs.plantuml)                 | vs code extension         | 2.10.2          |
| [LaTeX Math for Markdown](https://marketplace.visualstudio.com/items?itemName=goessner.mdmath) | vs code extension         | 2.3.7           |
| [GraphViz](http://www.graphviz.org/download/)                                                  | for PlantUML local render | 2.38            |
| [java](https://www.java.com/en/download/)                                                      | for PlantUML local render | 8 Updata 201    |

## Tooling

- consider the use of clang-tidy
- need for virtual machine ([VirtualBox](https://www.virtualbox.org/) ?), for testing for compilation and tools, for testing the game on previous version of Windows and Linux

### BuildTask

engine is one of the "pilot" projects using BuildTask (temporary name, to be redefined)

- consider the flag /MP for MSVC multiprocess compilation
- consider the flag /c to not link