# R-Type

A networked clone of the classic horizontal shoot'em up **R-Type**, built on a small game
engine of our own (ECS, fixed-timestep loop, UDP networking). Up to 4 players fight waves of
Bydos on one authoritative server.

> Epitech B-CPP-500 project. Status: the game is playable end to end. The authoritative server
> and the client run a full loop (Bydos enemies, player missiles, lives, score), with client-side
> prediction, reconciliation and interpolation. Part 2 track: **Advanced Networking** (see
> [design decisions](docs/decisions.md) for what is done and what is not).

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

Supported platforms: Linux, Windows and macOS are all built in CI (GitHub Actions matrix), but
only Linux has been run by hand. Windows and macOS builds are not validated by playing, see
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

Quick start: build, then start one server and one client per player (up to 4).

```sh
./r-type_server [PORT]                         # UDP port, default 4242
./r-type_client [HOST [PORT]] [--name NAME]    # defaults: 127.0.0.1, 4242, Player
```

Controls: **arrow keys** or **WASD** to move, **Space** to shoot, **Escape** to quit.
Keys, resolution, fullscreen, vsync, colour palette and high contrast are read from the
settings file `~/.config/r-type/settings.cfg`.

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

**[Antonin MEYNADIER](https://github.com/pastirami)** <br>
**[Victor RIPERT](https://github.com/Victor-ripert)** <br>
**[Valentin MINÉ](https://github.com/chezrush)**

## License

Released under the [MIT License](LICENSE).
