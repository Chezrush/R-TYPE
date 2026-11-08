# Protocol implementation

This page maps the [protocol RFC](rtype-protocol.md) to the code. Everything below lives in
`protocol/` (namespace `rtype::protocol`) except the `Gateway`, which is in `server/`. Nothing in
`protocol/` does I/O or reads a clock: datagrams and time are passed in, so every behavior is
tested without a network.

## Layers

```mermaid
flowchart TB
    subgraph server
        Server --> Gateway
    end
    subgraph protocol
        Gateway --> ServerHandshake
        Gateway --> Connection
        ClientHandshake --> Messages
        Connection --> ReliableSender
        Connection --> ReliableReceiver
        Connection --> ReceiveWindow
        Connection --> RttEstimator
        Connection --> MessageCodec
        MessageCodec --> PacketReader
        MessageCodec --> PacketBuilder
        PacketReader --> ByteOrder
        ServerHandshake --> CookieFactory
        ServerHandshake --> RateLimiter
        CookieFactory --> SipHash
    end
```

| RFC section | Code |
| --- | --- |
| 2 Conventions (big-endian, serial arithmetic, fixed point) | `ByteReader`, `ByteWriter`, `SerialNumber`, `FixedPoint` |
| 3 Datagram format, 4 Decoding procedure | `PacketReader` (cheap validation), `PacketBuilder` |
| 6 Messages | `Messages.hpp` (typed structs), `MessageCodec` (encode and decode) |
| 5.1 Sequence numbers and duplicates | `ReceiveWindow` |
| 5.2 Reliable ordered channel | `ReliableSender`, `ReliableReceiver` |
| 5.3 RTT | `RttEstimator` |
| 5 and 8.3 One side of a connection | `Connection` |
| 8.1 Handshake, anti-amplification | `ClientHandshake`, `ServerHandshake`, `CookieFactory`, `SipHash` |
| 8.2, 8.3, 11 Sessions, liveness, flood limits | `Gateway`, `RateLimiter` |

## How a datagram flows

Receiving, in `Connection::receive`:

1. `PacketReader::isValid` checks the header and every record (steps 1 to 5 of the decoding
   procedure). Any failure drops the whole datagram and changes nothing.
2. The session is compared, then `ReceiveWindow::check` rejects duplicates and too old datagrams.
3. Every reliable record is checked against the receive window of the reliable channel before
   anything is committed.
4. Only then the window, the acknowledgements and the RTT are updated, and the records are decoded
   with `MessageCodec::decodeBody` and handed to the game. Unknown types are skipped.

Sending, in `Connection::buildDatagram`: header with the current `ack` and `ack_bits`, then the
unreliable messages that fit, then the reliable records that are due (never sent, timed out, or
overtaken by three acknowledged datagrams). The result never exceeds 1200 bytes nor 64 records.

## Design choices

| Choice | Alternatives | Reason |
| --- | --- | --- |
| Cookie from SipHash-2-4 of the address and a 5 s slot | Table of pending hellos, plain hash, HMAC-SHA256 | No state before the client answers, and a keyed hash nobody can forge; SipHash is short, fast and has public test vectors. HMAC-SHA256 would need a crypto library for no gain here |
| Sticky-failure `ByteReader` | Exceptions, `std::optional` on every read | A truncated body can never read out of bounds, and the decoder stays `noexcept` and short |
| Typed `std::variant` messages | One class per message with virtual methods | Closed set, no heap, `std::visit` for encoding, equality for free in tests |
| `Connection` takes the time as a parameter | Reading the clock inside | Deterministic tests of retransmission and RTT, no sleep |
| One `Gateway` mutex | Lock-free structures | The network thread is the only hot caller, the others (simulation, tests) call rarely |

## Limits of this implementation

- `GameEncoder` (in `server/`) turns the game events into `spawn`, `destroy`, `score` and
  `game_state`, and sends a snapshot every `tick_rate / snapshot_rate` ticks with the priority
  of section 9.2. Every client gets the same snapshot, so `input_ack` stays 0 until the server
  applies inputs.
- Client prediction and interpolation (section 9.3) are client work and are not in the library.
- Confidentiality and integrity are out of scope of version 1 (section 11).
