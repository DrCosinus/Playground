guidelines:
- use of C++17, support MSVC (from version 19.16, maybe previous), Clang (from version 7.0?)
- using clang-format

- engine sub project should be extract from the playground repository to its own repository
- engine should be split in two: engine.exe and game.dll
  game.dll should be "hot" compilable
- implementing tileset (square, rectangle, hexagon, custom...)
- implementing something like libfmt...
- implementing something like moustache (debugging logs history, analysis facilities)

documentation:
- plantuml
- graphviz
- md + math extension

tooling:
- consider to excavate synapse:
  - split renderer (to be shared with other tools) & the message system
    renderer should be rely on an abstraction of DirectX/OpenGL/GDI or (why not?) the renderer of the engine
- consider the use of clang-tidy
- profiling/benching facilities
- need for virtual machine (openVM virtualbox ?), for testing for compilation and tools, for testing the game on previous version of Windows and Linux

engine is one of the "pilot" projects using BuildTask (temporary name, to be redefined)
- consider the flag /MP for MSVC multiprocess compilation
- consider the flag /c to not link