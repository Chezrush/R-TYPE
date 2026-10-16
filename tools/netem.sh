#!/usr/bin/env bash
# Simulate a degraded network on the loopback interface (Linux, needs root) to test the game
# under the conditions asked by the subject. On Windows, use "clumsy" instead.
#
#   sudo tools/netem.sh loss      # 2% packet loss
#   sudo tools/netem.sh dup       # 2% duplicated packets
#   sudo tools/netem.sh lag       # 150 ms latency (+/- 20 ms jitter)
#   sudo tools/netem.sh slow      # ~5 KB/s bandwidth (40 kbit/s)
#   sudo tools/netem.sh reset     # back to normal
set -euo pipefail

IFACE="${IFACE:-lo}"

case "${1:-}" in
    loss)  tc qdisc replace dev "$IFACE" root netem loss 2% ;;
    dup)   tc qdisc replace dev "$IFACE" root netem duplicate 2% ;;
    lag)   tc qdisc replace dev "$IFACE" root netem delay 150ms 20ms ;;
    slow)  tc qdisc replace dev "$IFACE" root tbf rate 40kbit burst 1600 latency 400ms ;;
    reset) tc qdisc del dev "$IFACE" root 2>/dev/null || true ;;
    *)     echo "usage: $0 {loss|dup|lag|slow|reset}  (IFACE=$IFACE)"; exit 1 ;;
esac
echo "netem '$1' applied on $IFACE"
