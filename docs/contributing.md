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

- C++20, formatted with `clang-format` (config in `.clang-format`), checked by `clang-tidy`.
- Headers live in `<module>/include/<module>/`, sources in `<module>/src/`, tests in
  `<module>/tests/`. Include public headers as `<engine/ecs/registry.hpp>`.
- Types and functions in `snake_case`, private members prefixed with `_`.
- Prefer RAII and the standard library. Wrap system APIs (sockets, threads) behind a small
  class of ours.
- Dependency direction: `engine` ← `protocol` ← `game` ← `server` / `client`. Never the
  other way round, and never R-Type specifics in `engine`.
- **Never trust network data.** Validate every length, count and range before using it.
- New dependency? Add it to `vcpkg.json`. Never copy third-party sources into the repo.

## Before opening a merge request

```sh
cmake --build --preset debug
ctest --preset debug
clang-format -i $(git ls-files '*.cpp' '*.hpp')
```

- Add or update tests for what you changed.
- Update the docs if behaviour changed, and the [protocol](protocol/rtype-protocol.md) if
  you touched `protocol/` (bump the protocol version for breaking changes).

## CI

The workflow in `.github/workflows/chocolatine.yml` builds and tests on Linux and Windows. It caches
vcpkg downloads and binaries so dependencies are not rebuilt on every push.
