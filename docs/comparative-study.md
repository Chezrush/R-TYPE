# Technical and comparative study

Each section lists the options we considered and why we chose one. Entries marked **to
complete** need the team's own measurements or conclusions before the defense.

## 1. Language: C++20

C++ is imposed by the subject. We use C++20 for defaulted comparisons, `std::remove_cvref_t`,
parenthesised aggregate initialisation (`emplace_component<position>(e, 1.f, 2.f)`) and
`std::span`-style generic code, while staying portable to GCC, Clang and MSVC.

## 2. Build system and dependencies

| Option | Pros | Cons |
| --- | --- | --- |
| **CMake + vcpkg (manifest)** | de-facto standard, presets, `vcpkg.json` pins deps, Windows + Linux | first configure is slow (builds deps) |
| CMake + Conan | very flexible, binary caching | extra profile/recipe files |
| CMake + CPM | tiny, source-only | recompiles everything, weak on big libs such as SFML |
| Meson / Xmake | pleasant syntax | smaller ecosystem for teammates |

**Choice: CMake + vcpkg.** Dependencies are declared in `vcpkg.json`, never copied in the
repo. *To complete:* pin a `builtin-baseline` once the team agrees on a vcpkg commit.

## 3. Graphics, window, input: SFML

| Option | Pros | Cons |
| --- | --- | --- |
| **SFML 3** | simple, 2D focused, window + audio + input in one, great vcpkg support | no built-in GUI |
| SDL2/3 | very mature, gamepad support | C API, more boilerplate |
| Raylib | very easy | C API, bigger scope |

**Choice: SFML**, confined to `client/src/window.cpp` so the choice can change cheaply.

## 4. Networking: UDP + Asio (hidden behind `udp_socket`)

| Question | Options | Decision |
| --- | --- | --- |
| Transport | TCP (ordered, reliable, head-of-line blocking) vs **UDP** (datagrams, lossy, low latency) | UDP, imposed by the subject and right for real-time state |
| API | raw BSD / Winsock sockets vs **Asio** | Asio: one portable API, already battle-tested. Wrapped in a pimpl so nothing else depends on it |
| Threading | one thread per client, vs one I/O thread + game thread with queues | I/O thread + bounded queues: simple, no locks in game code |
| Encoding | text (JSON) vs **hand-written binary** | binary: small, fixed-size, trivially bounds-checkable |

*To complete:* measured bandwidth per player, and the same measurement under
`tools/netem.sh loss|lag|slow`.

### Part 2 track: Advanced Networking

| Topic | Options | Decision and status |
| --- | --- | --- |
| Reliability over UDP | ENet / yojimbo (hide the mechanism) vs **own layer** (`seq`, `ack` + 32-bit mask, reliable ordered records) | Own layer, so that we can explain and measure it. **Done** |
| Data size | `float` / `u32` vs **quantization** (fixed-point `i16`, `u16` ids) | Quantized, snapshots at 20 Hz instead of 60 Hz. **Done** |
| Latency hiding | lockstep, rollback vs **prediction + reconciliation** (`input_ack`) and **interpolation** of other entities | Prediction, reconciliation and interpolation. **Done**, the server stays authoritative |
| Payload size | **compression** (LZ4, zlib), delta snapshots | **Not done**: absolute snapshots repair themselves after a loss, and the size target is met without it |
| Several games | **lobbies / rooms**, accounts, chat | **Not done**: one game per server |
| Fairness | **lag compensation** on the server | **Not done** |

Measurements of bandwidth and of responsiveness under degraded networks are still to be
published.

## 5. Algorithms and data structures

| Need | Chosen | Why / alternatives |
| --- | --- | --- |
| Entity storage | **ECS with sparse arrays** | composition over inheritance, decoupled systems. Alternatives: OOP hierarchy (rigid), archetypes (faster but more complex) |
| Multi-component iteration | custom `zipper` | no boilerplate in systems, skips missing components. Cost measured in [section 8](#8-ecs-measurements) |
| Subsystem communication | typed `event_bus` (mediator) | no direct dependencies between subsystems |
| Game pacing | fixed timestep (60 Hz) with clock-based sleeping | deterministic, independent from CPU speed |
| Collisions | AABB, O(missiles × enemies) | fine for ≤ ~150 entities. Spatial hashing if the counts grow |
| Entity ids on the wire | monotonically increasing `net_id` | registry indices are recycled, so a late packet could otherwise hit the wrong entity |

## 6. Storage

Part 1 keeps everything in memory: the world is rebuilt at each start and nothing needs to
survive a restart. If Part 2 adds accounts or a scoreboard we will compare SQLite (embedded,
transactional, zero-ops) with flat files (simple but no concurrency control) and a client-
server database (overkill for this scale). The chosen Part 2 track (Advanced Networking) does
not include accounts or a scoreboard yet, so nothing is persisted and this comparison stays
open.

## 7. Security

| Threat | Mitigation | Where |
| --- | --- | --- |
| Malformed or truncated packets, bad lengths | strict decoder: exact length, bounded counts, no over-read | `protocol/src/packet.cpp` |
| NaN / absurd coordinates | rejected on decode | same |
| Memory exhaustion by packet flood | bounded queues, max 512 datagrams per tick | `concurrent_queue`, `server.cpp` |
| Oversized datagrams | dropped by the socket layer | `udp_socket.cpp` |
| Spoofed or unknown senders | only known endpoints can send `input`; max 4 clients | `server.cpp` |
| Cheating clients | server authority: clients send only pressed keys | architecture |
| Crashing client | 5 s silence timeout, other players notified | `server.cpp` |

Known gaps: no encryption or authentication, a spoofed source address can hijack a session,
no rate limiting per client. *To complete:* long-term monitoring plan (log malformed-packet
counters, run the fuzz tests in CI, dependency updates through vcpkg baseline bumps).

## 8. ECS measurements

Measured on 2026-10-25 with the `rtype_engine_bench` program (`engine/bench/`), built from the
`main` branch at commit `687e236`.

### Conditions

| Item | Value |
| --- | --- |
| CPU | Intel Core i7-9700K, 3.6 GHz, 8 cores, 1 thread per core |
| System | Linux 6.18 (WSL2 on a Windows host), nothing else deliberately stopped |
| Compiler | GCC 13.3.0, CMake `Release` (`-O3 -DNDEBUG`) |
| Build | `-DRTYPE_BUILD_BENCHMARKS=ON`, tests and demos off |
| Run | `rtype_engine_bench <entities> 30`, one process per entity count |
| Repetitions | 30 per case, the first one is not discarded |
| Timer | `std::chrono::steady_clock`, one reading around each repetition |
| Components | `Position` and `Velocity`, two `float` each |

To reproduce:

```sh
cmake -S . -B build/bench -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
      -DRTYPE_BUILD_BENCHMARKS=ON
cmake --build build/bench --target rtype_engine_bench
build/bench/bin/rtype_engine_bench 100000 30
```

### What each case measures

| Case | Measured (per repetition) |
| --- | --- |
| `zipper` | iterate the entities owning `Position` and `Velocity` and update the position |
| `spawn` | `spawnEntity` N times on an empty registry, no component |
| `kill` | `killEntity` on those N entities, no component |
| `cmd-record` | record one `add<Position>` command per entity in a `CommandBuffer` |
| `cmd-flush` | flush those N commands into the registry |
| `scheduler` | `Scheduler::run` with three systems in three phases over N entities |

The setup of each repetition (building the world, spawning the entities that `kill` and
`cmd-*` need) is outside the timed section.

### Results

Mean time in nanoseconds per entity (or per command), lower is better.

| Case | 1 000 | 10 000 | 100 000 | 1 000 000 |
| --- | ---: | ---: | ---: | ---: |
| `zipper` | 1.2 | 1.2 | 2.3 | 3.5 |
| `spawn` | 7.4 | 7.6 | 9.1 | 8.8 |
| `kill` | 8.8 | 8.7 | 11.0 | 10.0 |
| `cmd-record` | 19.9 | 19.1 | 21.2 | 19.7 |
| `cmd-flush` | 40.1 | 56.4 | 63.4 | 57.2 |
| `scheduler` | 18.2 | 20.8 | 23.0 | 22.7 |

The same data as milliseconds for one repetition at 100 000 entities:

| Case | Best (ms) | Mean (ms) |
| --- | ---: | ---: |
| `zipper` | 0.126 | 0.225 |
| `spawn` | 0.735 | 0.914 |
| `kill` | 0.831 | 1.097 |
| `cmd-record` | 1.818 | 2.121 |
| `cmd-flush` | 5.574 | 6.340 |
| `scheduler` | 1.879 | 2.296 |

### Reading the results

- **Cost per frame.** A frame at 60 Hz lasts 16.7 ms. The scheduler with three systems takes
  18 µs for 1 000 entities, about 0.1 % of that budget, and 2.3 ms for 100 000 entities. The
  game is expected to hold about 150 entities (section 5), so the ECS is not a bottleneck at
  that scale.
- **Iteration.** The `zipper` stays at about 1.2 ns per entity up to 10 000 entities and
  rises to 3.5 ns at one million. The data of a million entities is larger than the CPU
  caches, which is the likely reason, but we did not verify it.
- **Scheduler against raw iteration.** Each system of the scheduler costs about 7 ns per
  entity, against 1.2 ns for the raw `zipper` loop, although the `probe` and `wrap` systems
  do almost nothing. The typed adapter (indexed zipper, then `entityAt` to rebuild the handle
  of each entity) is where the difference comes from. We did not profile it, so we do not know
  which part dominates.
- **Command buffer.** Recording a command costs about 20 ns, flushing it 40 to 63 ns. Each
  command is a separately allocated object applied through a virtual call, then goes through
  the registry lookup. The flush is the expensive half; this was not profiled either.
- **Spawn and kill.** About 8 to 11 ns each. These runs have no component registered, so the
  cost of removing the components of a killed entity (one virtual `erase` per registered
  storage) is **not** included.

### Limits of these numbers

- One machine, run under WSL2: the scheduling noise of the host is not controlled. The mean
  is noticeably higher than the best time from 100 000 entities (`zipper`: 0.126 ms best,
  0.225 ms mean), so we read the `ns` figures as orders of magnitude, not as exact values.
- No alternative was benchmarked. We measured the cost of our own choices, not a gain over
  an archetype storage, a dependency graph or another command encoding.
- Entities have two components and the same shape, which is the best case for the `zipper`.
- The benchmark runs in one thread and the data stays in the cache for small counts.
