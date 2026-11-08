# Getting started

## 1. Install the tools

- A C++20 compiler (GCC 11+, Clang 14+, MSVC 2022), CMake 3.21+, Ninja.
- vcpkg, then export `VCPKG_ROOT`:

```sh
git clone https://github.com/microsoft/vcpkg ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh          # bootstrap-vcpkg.bat on Windows
export VCPKG_ROOT=~/vcpkg
```

- macOS: install the Xcode command line tools (`xcode-select --install`), then
  `brew install cmake ninja pkg-config python@3.12`. vcpkg needs Python 3.10 or newer to build
  some dependencies, and the system Python of macOS is older: put the Homebrew one first in your
  `PATH` (`export PATH="$(brew --prefix python@3.12)/libexec/bin:$PATH"`). Nothing else is needed,
  OpenGL and the windowing system are provided by macOS.
- Linux only: the development packages SFML needs to open a window (X11, OpenGL, udev,
  FreeType...). The vcpkg output tells you which ones are missing.

## 2. Build and test

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The presets are `debug`, `release` and `server-only` (everything except the client, for a
machine without a display). The `Makefile` wraps them: `make` (release, copies the two binaries
to the repository root), `make debug`, `make server-only`, `make tests_run` (debug build and
tests) and `make docs` (`mkdocs serve`, needs `pip install mkdocs-material`).

## 3. Run

```sh
build/debug/bin/r-type_server [PORT]                       # default 4242
build/debug/bin/r-type_client [HOST [PORT]] [--name NAME]  # defaults 127.0.0.1, 4242, Player
```

Controls: arrow keys or WASD to move, Space to shoot, Escape to quit. Settings (keys,
resolution, fullscreen, vsync, palette, high contrast) are in `~/.config/r-type/settings.cfg`.

### A 2-client game on one machine

1. Terminal 1: `build/debug/bin/r-type_server 4242`
2. Terminal 2: `build/debug/bin/r-type_client 127.0.0.1 4242 --name Alice`
3. Terminal 3: `build/debug/bin/r-type_client 127.0.0.1 4242 --name Bob`

Each client opens its own window, up to 4 players. Stop the server with Ctrl+C. Killing one
client does not stop the game: the others are told that the player left.

## 4. Test under bad network conditions

Linux (needs root, acts on the loopback interface, so run the server and the clients on the
same machine as above):

```sh
sudo tools/netem.sh loss    # 2% packet loss
sudo tools/netem.sh dup     # 2% duplicated packets
sudo tools/netem.sh lag     # 150 ms latency (+/- 20 ms jitter)
sudo tools/netem.sh slow    # ~5 KB/s
sudo tools/netem.sh reset   # back to normal
```

Set `IFACE=eth0` to act on another interface. Always run `reset` when you are done. On Windows
use [clumsy](https://jagt.github.io/clumsy/).

## Windows and macOS notes

CI builds the project on Linux, Windows and macOS, but only Linux has been run by hand
(server, clients, `netem`). The code is written to be portable (Asio, SFML, standard C++), so
problems on the other two systems are most likely to show up when playing: report them.
