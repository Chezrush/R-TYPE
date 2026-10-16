# R-Type

A networked clone of the classic horizontal shoot'em up **R-Type**, built on a small game
engine of our own (ECS, fixed-timestep loop, UDP networking). Up to 4 players fight waves of
Bydos on one authoritative server.

> Epitech B-CPP-500 project. Status: **Part 1** (architecture + first playable prototype).

## Requirements

| Tool | Version |
| --- | --- |
| C++ compiler | GCC 11+, Clang 14+ or MSVC 2022 (C++20) |
| CMake | 3.21+ |
| Ninja | any recent |
| [vcpkg](https://github.com/microsoft/vcpkg) | `VCPKG_ROOT` environment variable set |

Dependencies (Asio, SFML) are declared in `vcpkg.json` and fetched automatically at configure
time. Nothing needs to be installed system-wide besides the compiler and the usual graphics
libraries (OpenGL / X11 on Linux).

Supported platforms: Linux (client and server). Windows is a target (cross-play), see
[docs/guides/getting-started.md](docs/guides/getting-started.md).

## Build

```sh
make                          # builds ./r-type_server and ./r-type_client (release)
make tests_run                # debug build + unit tests

# or directly with CMake presets:
cmake --preset debug          # configure (downloads and builds the dependencies)
cmake --build --preset debug  # build
ctest --preset debug          # run the unit tests
```

Server only (no SFML needed, handy on a headless machine): use the `server-only` preset.

Binaries end up in `build/<preset>/bin/`: `r-type_server` and `r-type_client`.

## Play

```sh
./r-type_server 4242                 # UDP port, default 4242
./r-type_client 127.0.0.1 4242       # one per player, up to 4
```

Controls: **arrow keys** or **WASD** to move, **Space** to shoot.

## Project layout

| Directory | Role |
| --- | --- |
| `engine/` | Generic engine: ECS, game loop, event bus, UDP abstraction. Knows nothing about R-Type. |
| `protocol/` | Binary wire format shared by client and server (with strict validation). |
| `game/` | R-Type components, systems and rules. |
| `server/` | `r-type_server` executable. |
| `client/` | `r-type_client` executable (SFML). |
| `assets/` | Sprites, sounds, levels. |
| `docs/` | Developer documentation, protocol RFC, comparative study. |
| `tools/` | Helper scripts (network degradation with `netem`). |

## Documentation

Start with [docs/index.md](docs/index.md). Browse it as a website with
`pip install mkdocs-material && mkdocs serve`.

- [Architecture](docs/architecture/overview.md)
- [Network protocol](docs/protocol/rtype-protocol.md)
- [Comparative study](docs/comparative-study.md)
- [Accessibility](docs/accessibility.md)
- [Contributing](docs/contributing.md)

## Authors

_To fill in: team members and contacts._

## License

_To fill in._
