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

On a machine without a display, `--preset server-only` builds everything except the client.

## 3. Run

```sh
build/debug/bin/r-type_server 4242
build/debug/bin/r-type_client 127.0.0.1 4242   # repeat in other terminals, up to 4 players
```

## 4. Test under bad network conditions

Linux (needs root, acts on the loopback interface):

```sh
sudo tools/netem.sh loss    # 2% packet loss
sudo tools/netem.sh lag     # 150 ms latency
sudo tools/netem.sh slow    # ~5 KB/s
sudo tools/netem.sh reset
```

On Windows use [clumsy](https://jagt.github.io/clumsy/).

## Windows notes

The code is written to be portable (Asio, SFML, standard C++), but the Windows build is not
yet validated by CI jobs that run the game. Check it early: cross-platform problems are much
cheaper to fix at the start of the project.
