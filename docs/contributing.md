# Contributing

## Git workflow

- `main` is always buildable and playable. Nobody pushes to it directly.
- One **issue** per task, one **feature branch** per issue: `feat/<topic>`, `fix/<topic>`,
  `docs/<topic>`.
- Open a **merge request** early, get at least one review, then squash or rebase-merge.
- Commit messages: imperative mood, a short subject, a body that explains *why*.
  `Add collision system for missiles` rather than `fixed stuff`.
- Tag milestones: `v0.1-part1` for the first delivery, `v1.0-final` for the last one.

## Code conventions

The code follows the Epitech C++ Coding Style. `.clang-format` and `.clang-tidy` encode
most of it, and the CI checks both on every pull request.

- C++20, formatted with `clang-format` (config in `.clang-format`), checked by `clang-tidy`.
- Every `.cpp`, `.hpp`, `Makefile` and `CMakeLists.txt` starts with the Epitech header.
- Files and folders of C++ code in PascalCase (`Registry.hpp`, `Engine/Core/`), named after
  the class they define. Headers live in `<module>/include/<Module>/`, sources (`.cpp`
  only) in `<module>/src/`, tests in `<module>/tests/`.
- A header only declares: function bodies, even one-line accessors, go in one `.cpp` per
  topic (`= default` stays in the header).
- Types in PascalCase, functions, methods and variables in camelCase, private attributes
  prefixed with `_`, macros, global constants and enum values in UPPER_CASE.
- 4 spaces, 80 columns, one statement per line, opening brace at the end of the line except
  for functions, no `using namespace`, no `#pragma once` (use `#ifndef` include guards).
- Functions of 20 lines at most, 5 arguments at most, no comment inside a function body.
- Prefer RAII and the standard library, smart pointers instead of `new` / `delete`, `const`
  and `nullptr` wherever they apply. Wrap system APIs (sockets, threads) behind a small
  class of ours.
- Dependency direction: `engine` <- `protocol` <- `game` <- `server` / `client`. Never the
  other way round, and never R-Type specifics in `engine`.
- **Never trust network data.** Validate every length, count and range before using it.
- New dependency? Add it to `vcpkg.json`. Never copy third-party sources into the repo.

## Before opening a merge request

```sh
cmake --build --preset debug
ctest --preset debug
clang-format -i $(git ls-files '*.cpp' '*.hpp')
clang-tidy -p build/debug $(git ls-files '*.cpp')
```

- Add or update tests for what you changed. Unit tests use Catch2: declare an executable
  with `rtype_add_catch_test(name sources...)` in the module's `CMakeLists.txt` (see
  `engine/tests/`), each `TEST_CASE` then shows up as its own CTest test.
- Update the docs if behaviour changed, and the [protocol](protocol/rtype-protocol.md) if
  you touched `protocol/` (bump the protocol version for breaking changes).

## CI

The workflow in `.github/workflows/chocolatine.yml` builds and tests on Linux and Windows. It caches
vcpkg downloads and binaries so dependencies are not rebuilt on every push.
