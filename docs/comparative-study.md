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

*To complete:* measured bandwidth per player at 60 Hz, and the same measurement under
`tools/netem.sh loss|lag|slow`.

## 5. Algorithms and data structures

| Need | Chosen | Why / alternatives |
| --- | --- | --- |
| Entity storage | **ECS with sparse arrays** | composition over inheritance, decoupled systems. Alternatives: OOP hierarchy (rigid), archetypes (faster but more complex) |
| Multi-component iteration | custom `zipper` | no boilerplate in systems, skips missing components |
| Subsystem communication | typed `event_bus` (mediator) | no direct dependencies between subsystems |
| Game pacing | fixed timestep (60 Hz) with clock-based sleeping | deterministic, independent from CPU speed |
| Collisions | AABB, O(missiles × enemies) | fine for ≤ ~150 entities. Spatial hashing if the counts grow |
| Entity ids on the wire | monotonically increasing `net_id` | registry indices are recycled, so a late packet could otherwise hit the wrong entity |

## 6. Storage

Part 1 keeps everything in memory: the world is rebuilt at each start and nothing needs to
survive a restart. If Part 2 adds accounts or a scoreboard we will compare SQLite (embedded,
transactional, zero-ops) with flat files (simple but no concurrency control) and a client-
server database (overkill for this scale). *To complete if Part 2 needs persistence.*

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
