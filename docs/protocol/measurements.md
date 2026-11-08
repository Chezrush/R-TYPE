# Network measurements

This page gives the measured bandwidth and the behaviour of the reliable channel
under loss, duplication and latency. The numbers come from `tools/NetMeasure.cpp`
(target `rtype_net_measure`), run once on the development machine (Linux, WSL2).
Nothing here is estimated: re-run the tool to reproduce it.

## Method

The tool prints three Markdown tables.

1. **Snapshot size.** The size of one `snapshot` datagram is computed with the real
   `MessageCodec::recordSize` plus the 16 byte header, for 1, 10, 30 and 60 entities
   (6 bytes per entity). 28 bytes of IPv4 and UDP headers are added to get the size
   on the wire. The bandwidth per client is that size multiplied by 20 Hz (the
   snapshot rate of the server) and by 60 Hz (the alternative of one snapshot per
   tick), compared with the 5 KB/s target of the subject.
2. **Reliable channel on a simulated link.** Two real `Connection` objects exchange
   datagrams through a deterministic link (`std::mt19937` with fixed seeds, loss,
   duplication and a fixed one-way latency). The clock is virtual (steps of 5 ms),
   nothing sleeps. The sender queues one reliable `score` message every 50 ms
   (200 messages), both sides send a datagram at 20 Hz, and the sender also sends
   as soon as a record is due again. Latency is the time between the call to
   `queueReliable` and the delivery to the game on the other side, in order. A
   retransmission is a reliable record whose `rel_id` was already seen in a
   datagram sent by the sender. The overhead is
   `(bytes on the wire, both directions - 200 x 5 B of payload) / payload`.
3. **Real loopback UDP.** A real `rtype::server::Server` (port 0, 60 Hz simulation,
   20 Hz snapshots) is started, 1 to 4 peers built on the protocol library connect
   through the handshake, then send an `input` datagram every 16 ms for 3 seconds
   while holding right and shoot. The bytes received and sent by each peer are
   counted at the socket (UDP payload only, without the IP and UDP headers).

## Commands

```sh
cmake --preset server-only
cmake --build --preset server-only --target rtype_net_measure
./build/server-only/bin/rtype_net_measure
```

The server log lines are printed on the same stream; filter them with
`grep -v '^\[server'`.

## Results

### Snapshot size and bandwidth per client

| Entities | Datagram (B) | With IP+UDP (B) | 20 Hz (B/s) | 60 Hz (B/s) | 20 Hz <= 5 KB/s | 60 Hz <= 5 KB/s |
|---|---|---|---|---|---|---|
| 1 | 32 | 60 | 1200 | 3600 | yes | yes |
| 10 | 86 | 114 | 2280 | 6840 | yes | no |
| 30 | 206 | 234 | 4680 | 14040 | yes | no |
| 60 | 386 | 414 | 8280 | 24840 | no | no |

Sending a snapshot every tick (60 Hz) triples the bandwidth and exceeds 5 KB/s from
10 entities. At 20 Hz the budget holds up to about 30 entities, and is exceeded with
60 (the cap is `MAX_SNAPSHOT_ENTITIES` = 128). This is the reason why a delta
snapshot or a relevance filter is the next step of the networking track.

### Reliable delivery of 200 messages (in order)

| Loss % | Dup % | One-way ms | Delivered | Mean ms | Max ms | Retransmitted records | Wire bytes | Overhead % |
|---|---|---|---|---|---|---|---|---|
| 0 | 0 | 0 | 200/200 | 0.0 | 0.0 | 0 | 8400 | 740.0 |
| 2 | 2 | 0 | 200/200 | 3.0 | 100.0 | 8 | 8480 | 748.0 |
| 10 | 2 | 0 | 200/200 | 16.0 | 200.0 | 41 | 8810 | 781.0 |
| 30 | 2 | 0 | 200/200 | 83.5 | 455.0 | 148 | 10760 | 976.0 |
| 0 | 0 | 75 | 200/200 | 75.0 | 75.0 | 0 | 8432 | 743.2 |
| 2 | 2 | 75 | 200/200 | 98.0 | 405.0 | 4 | 8536 | 753.6 |
| 10 | 2 | 75 | 200/200 | 192.3 | 705.0 | 21 | 8930 | 793.0 |
| 30 | 2 | 75 | 200/200 | 566.0 | 1175.0 | 88 | 10800 | 980.0 |
| 0 | 0 | 150 | 200/200 | 150.0 | 150.0 | 4 | 8536 | 753.6 |
| 2 | 2 | 150 | 200/200 | 202.2 | 655.0 | 8 | 8640 | 764.0 |
| 10 | 2 | 150 | 200/200 | 464.9 | 1205.0 | 24 | 9040 | 804.0 |
| 30 | 2 | 150 | 200/200 | 1305.0 | 3055.0 | 95 | 10966 | 996.6 |

Reading the table:

- All 200 messages are delivered, in order and exactly once, in every scenario,
  including 30% loss with 2% duplication.
- The "Dup %" column is 0 on the lossless rows and 2 on the others.
- With 150 ms one-way latency and no loss, 4 records are sent twice: the first
  retransmission timeout is the 100 ms minimum, below the 300 ms round trip, until
  the round trip estimate converges. The delivery time itself is not affected.
- Loss mostly hurts the maximum: a lost record waits for the retransmission
  timeout, `max(2 x srtt, 100 ms)`, or for three newer acknowledged datagrams.
- The overhead is large in relative terms because the test payload is tiny (5 bytes
  per message) and each 20 Hz datagram carries a 16 byte header, in both directions.
  It is the cost of the acknowledgements, not of the reliability itself, and it is
  not representative of the snapshot traffic where the payload dominates.

### Real loopback UDP (3 s, inputs every 16 ms)

| Peers | Server to client (B/s per peer) | Client to server (B/s per peer) | Datagrams/s per peer | Max entities in a snapshot |
|---|---|---|---|---|
| 1 | 2321 | 1426 | 82 | 9 |
| 2 | 3063 | 1418 | 82 | 17 |
| 3 | 3791 | 1395 | 81 | 25 |
| 4 | 4535 | 1418 | 82 | 33 |

The server sends each client about 82 datagrams per second (20 Hz snapshots plus
the acknowledgements and the other records). With 4 players and up to 33 entities in
a snapshot, the measured 4535 B/s per client stays under 5 KB/s (UDP payload, without
the 28 bytes of IP and UDP headers per datagram, which would add about 2.3 KB/s at
82 datagrams per second). Client to server stays near 1.4 KB/s: the client sends
about 62 input datagrams per second at 16 ms, each with a 16 byte header.

## Limits of these measurements

- The reliable-channel table is an **in-process simulation**: the link is a model
  (fixed latency, no jitter, no reordering, independent random loss), not a real
  network. One seed is used per direction, so the figures are one deterministic
  run, not an average over many.
- The loopback run uses real UDP sockets and a real server, but over the loopback
  interface: no loss, no latency, no bandwidth limit. It measures the traffic the
  protocol produces, not how it behaves under degradation.
- `tc` and `netem` could not be used for these runs (root access was not available),
  so loss and latency were **not** applied to the real server. The behaviour of the
  game under the three conditions of the subject (2% loss or duplication, latency
  above 150 ms, about 5 KB/s) still has to be measured with the commands below.
- The loopback figures count UDP payload bytes; the wire cost is 28 bytes more per
  datagram.
- The load is short (3 s) and synthetic: peers hold two keys, so the number of
  entities comes from the missiles of the players, with no enemy waves counted
  separately. Timings come from a single run on one machine and vary slightly
  between runs (the first loopback column changed from 2277 to 2321 B/s between two
  runs).
- Client-side prediction and reconciliation are not measured here.

## Repeating the measurements on a degraded network

On Linux, `tools/netem.sh` applies the conditions of the subject on the loopback
interface (root required). On Windows, use `clumsy`.

```sh
sudo tools/netem.sh loss     # 2% loss
sudo tools/netem.sh dup      # 2% duplication
sudo tools/netem.sh lag      # 150 ms +/- 20 ms
sudo tools/netem.sh slow     # about 5 KB/s
sudo tools/netem.sh reset    # back to normal
```

A suggested procedure, one condition at a time:

```sh
./build/server-only/bin/r-type_server &
sudo tools/netem.sh loss
./build/server-only/bin/rtype_net_measure | tail -8
sudo tools/netem.sh reset
```

Since the loopback interface carries both directions, the delay and the loss of
`netem` apply to the traffic of the server and of the clients. Run the real client
(`r-type_client`) in the same conditions to check the game feel, and record the
result here.
