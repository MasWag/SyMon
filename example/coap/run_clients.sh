#!/usr/bin/env bash
# run_clients.sh — reset state, run the multi-agent chaos, then show
# that the trace grew and that agents interleave (different client
# ports on the first few events). Convenience wrapper around
# run_chaos.sh for demo runs.
set -eo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

echo "=== resetting trace + truth logs ==="
rm -f trace.txt truth.*.log

echo
echo "=== launching multi-agent chaos (~15s) ==="
bash run_chaos.sh 2>&1 | tail -12

echo
echo "=== trace size + one peek at the interleaving ==="
wc -l trace.txt
echo "--- first 6 events (notice the client ports differ) ---"
head -6 trace.txt
