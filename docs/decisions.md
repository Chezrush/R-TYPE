# Design decisions

Short record of the architecture choices made for the ECS, with the alternatives that were
considered. The long form belongs in the [comparative study](comparative-study.md).

Status: **decided** means implemented or ready to implement, **direction** means the
principle is chosen and the details are still to be settled.

## ECS

| Topic | Chosen | Alternatives | Why | Status |
|---|---|---|---|---|
| Naming | PascalCase types, camelCase functions | snake_case of the bootstrap | The Epitech C++ Coding Style is mandatory and overrides the bootstrap | decided |
| Component storage | Abstract `IStorage`, implemented by `SparseArray<C>` | `std::any` + per-type erase functions (bootstrap) | One virtual call per entity removal is negligible, the code is simpler to read and test, and the same interface can later carry serialization for network replication | decided |
| Entity identity | Index + generation | Plain index (bootstrap) | A reused index must not be mistaken for the entity that died. Matters most for late network messages | decided |
| Network identity | `NetId` component, separate from the entity | Send the entity value | The wire protocol must not depend on the internal layout of the ECS | decided |
| Structural changes while iterating | Deferred `CommandBuffer` applied at the end of a phase | Forbid them, or copy the arrays before iterating | Keeps iteration safe without locks or copies, and lets systems spawn and kill freely | decided |
| Systems | Named phases, declared read and write access, a scheduler | Flat list run in registration order (bootstrap) | Adding a system does not require touching the others, ordering is explicit, and parallel execution becomes possible later | direction |
| Threading | Registry owned by the game-loop thread, network threads feed it through a queue | Locks around the registry, one registry per thread | No lock in the hot path, deterministic updates, and the server loop never blocks on clients. Parallel systems only if measurements justify them | direction |
| Component registration | Explicit `registerComponent<C>()` before first use | Lazy registration on the first `addComponent` or `getComponents` | An allocation never hides inside an access, a typo or a forgotten type is detected instead of silently creating an empty storage, and the stale-handle and unregistered-type policy stays simple to state and test. Same choice as the bootstrap | decided |
| Registry storage table | `std::type_index` to `std::unique_ptr<IStorage>`, typed access by `static_cast` after the lookup | `std::any` per type (bootstrap); a per-type integer id handed out by a static counter | The lookup is the only place that knows the type, one owner per storage, and no `any_cast` on every access. A static counter would be faster but couples all registries to one global state, which breaks tests that build several registries | direction |

## Code organization

| Topic | Chosen | Alternatives | Why | Status |
|---|---|---|---|---|
| Content of a header | Declarations only; function bodies, even one-line accessors, live in one `.cpp` per topic (`= default` stays in the header) | Inline and `constexpr` bodies in the header | The header reads as a contract, a change of body does not recompile every includer, and the bodies of a topic are in one place | decided |
| Bodies of class templates (`SparseArray<C>`) | A `.tpp` file next to the header, included on its last line | Explicit instantiation in a `.cpp`; bodies in the `.hpp`; C++20 modules | See below | decided |

Why a `.tpp` for templates. A template is compiled only for the types it is used with, so its
bodies must be visible wherever it is used; a body that stays in a `.cpp` produces
"undefined reference" at link time.

- **`.tpp` included by the header (chosen).** The `.hpp` stays a list of declarations and the
  bodies are in one separate file, which keeps the rule above. It works for any component
  type, including the ones the game adds later without touching the engine. Cost: the bodies
  are compiled again in every file that includes the header, which is negligible at this size.
- **Explicit instantiation (`template class SparseArray<Position>;` in a `.cpp`).** Faster to
  compile and hides the bodies, but the list of types must be known when the engine is built.
  The engine must not know the R-Type components (it has to be extractable as a library), so
  each new component of the game would force an edit of the engine. Rejected.
- **Bodies in the `.hpp`.** The usual way in C++ and the simplest, but it breaks the rule that
  a header only declares, and it makes headers long and hard to read. Rejected by the team.
- **C++20 modules.** Would remove the header/body split altogether, but the support in CMake,
  the compilers and the CI on Windows is still uneven, and it would add a risk to a project
  with a hard deadline. Revisit if the toolchain matures.

## CI

| Topic | Chosen | Alternatives | Why | Status |
|---|---|---|---|---|
| Dependency cache | The build workflow also runs on every push to `main`, and vcpkg is pinned to a commit | Cache filled only by pull requests; latest vcpkg at each run; external binary cache | A pull request can only read the caches of its own branch and of `main`, so without a run on `main` every pull request rebuilt SFML, Asio and Catch2. Measured result: the GitHub Actions of a pull request went from 5 to 10 minutes down to less than a minute in general, and always under two minutes | decided |
| Superseded runs of the same pull request | A new push cancels the previous run (`concurrency` group per ref) | Let every run finish | Saves runner minutes. Runs on `main` are never cancelled, otherwise a cancelled run would not save its caches | decided |

## Tests

| Topic | Chosen | Alternatives | Why |
|---|---|---|---|
| Test framework | Catch2 v3 | Criterion (recommended by the school), GoogleTest, doctest | Criterion targets C and is awkward with templates. Catch2 offers `TEMPLATE_TEST_CASE` to run the same test on many component types, `SECTION` for shared setup, and integrates with CTest |

## Network protocol

The wire format is specified in the [protocol RFC](protocol/rtype-protocol.md).

| Topic | Chosen | Alternatives | Why | Status |
|---|---|---|---|---|
| Serialization | Hand-written binary format, big-endian, fixed-point integers | Protobuf, FlatBuffers, Cap'n Proto; text formats (JSON) | The subject asks for a documented binary protocol, and bit packing, quantization and a strict decoder are easier to control by hand. No extra dependency. Text is too large for the bandwidth target | decided |
| Reliability | Own layer in the header: `seq`, `ack` + 32-bit mask, reliable ordered records | ENet, yojimbo, GameNetworkingSockets; TCP for events; no reliability | Libraries hide what Part 2 asks us to implement and to measure. A TCP side channel needs a strong justification and brings head-of-line blocking. Without reliability a lost `spawn` or kill is never repaired | decided |
| Header | 16 bytes: magic, version, flags, session, seq, ack, ack bits | Header of 6 bytes (type and length only) | Sequence numbers detect the duplicated and reordered datagrams of the `netem` tests and make acknowledgements possible. Adding them later would be a breaking change. The cost is about 0.3 KB/s | decided |
| Framing | Several records per datagram, each with a type and a length | One message per datagram | Reliable records can be re-sent inside any datagram and share the header. Unknown types can be skipped, so the protocol can grow without a new version | decided |
| Connection | Stateless cookie handshake, then a random `session` bound to the address | Accept any `hello` at once; session id only | Without the cookie a spoofed source fills the 4 slots and the server can be used for reflection. The cost is one round trip at connection time | decided |
| Numbers | Fixed-point `i16` (1/16 px) and `u16` entity ids | `float` and `u32` | Halves the size of an entity state, and there is no NaN or infinity to validate. 1/16 px is finer than a pixel on a 1280 x 720 field. Ids are unique among living entities only | decided |
| State updates | Sparse absolute snapshots at 20 Hz, velocity hint in `spawn` | Full snapshot at 60 Hz; delta against an acknowledged baseline | A full snapshot at 60 Hz is about 13 times over the 5 KB/s target. Absolute entries repair themselves after a loss, unlike deltas which need baseline tracking. Delta compression stays possible in a later version | decided |
| Enumerations | Open sets: unknown `kind`, `variant`, `reason` are passed to the game | Closed sets rejected by the decoder | New monsters, weapons and bosses (Part 2, advanced gameplay) need no protocol change | decided |
| Netcode | Client prediction with `input_ack`, interpolation of the others, redundant inputs | Lockstep; rollback | Lockstep stalls on the slowest link. Rollback needs a deterministic simulation on every client, which is too heavy for this schedule. Prediction keeps the server authoritative | direction |
| Confidentiality | None in version 1, `flags` bit reserved | DTLS, own encryption now | Out of the subject's scope for Part 1. The reserved bit and the version field allow adding it without breaking the format | direction |

## Rules applied to every decision

- Each choice is justified against at least one alternative.
- A choice that deviates from the bootstrap says why.
- Performance claims are backed by a measurement before they go in the final study.
