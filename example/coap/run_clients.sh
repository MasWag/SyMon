#!/usr/bin/env bash
# Reset state, run multi-agent chaos, peek at the interleaved trace.
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
