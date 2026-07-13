#!/usr/bin/env bash
# run_chaos.sh — launch session_server + N parallel session_chaos agents.
# Each agent gets its own truth log; trace.txt is the single shared trace
# the server writes (each event tagged with the agent's ephemeral port
# via remote.hostinfo). The session_ssn / session_order monitors key on
# (client, server, kid), so interleaved per-agent streams stay disjoint.

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Prefer a local venv if present, else system python.
if   [[ -x "./.venv/bin/python"  ]]; then PYTHON="./.venv/bin/python"
elif [[ -x "../.venv/bin/python" ]]; then PYTHON="../.venv/bin/python"
else                                      PYTHON="python3"
fi

if ! "$PYTHON" -c 'import aiocoap' 2>/dev/null; then
  echo "Error: aiocoap not importable via $PYTHON." >&2
  echo "       Install it into a virtualenv (recommended):" >&2
  echo "         python3 -m venv .venv && ./.venv/bin/pip install aiocoap" >&2
  echo "       Or install into your system Python: pip install aiocoap" >&2
  exit 1
fi

SERVER_LOG=/tmp/session_chaos_server.log

# Fleet + agent tunables — all overridable via env vars.
N_AGENTS="${N_AGENTS:-5}"
STAGGER_STEP="${STAGGER_STEP:-0.2}"

# Per-agent chaos knobs (forwarded to session_chaos.py). Empty ⇒
# use session_chaos.py's own DEFAULT_* fallback for that knob.
DURATION="${DURATION:-}"
RATE="${RATE:-}"
VIOLATION_PROB="${VIOLATION_PROB:-}"
CLEAN_BURST="${CLEAN_BURST:-}"

extra_args=()
[[ -n "$DURATION"       ]] && extra_args+=(--duration       "$DURATION")
[[ -n "$RATE"           ]] && extra_args+=(--rate           "$RATE")
[[ -n "$VIOLATION_PROB" ]] && extra_args+=(--violation-prob "$VIOLATION_PROB")
[[ -n "$CLEAN_BURST"    ]] && extra_args+=(--clean-burst    "$CLEAN_BURST")

existing=$(lsof -ti :5683 2>/dev/null || true)
if [[ -n "$existing" ]]; then
  echo "[chaos] killing leftover :5683 (PID $existing)"
  kill $existing 2>/dev/null || true
  sleep 0.3
fi

echo "[chaos] launching session_server (log: $SERVER_LOG)"
"$PYTHON" session_server.py --trace trace.txt >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!
trap "kill $SERVER_PID 2>/dev/null || true" EXIT

for _ in 1 2 3 4 5 6 7 8 9 10; do
  lsof -i :5683 >/dev/null 2>&1 && break
  sleep 0.2
done

echo "[chaos] server ready; launching $N_AGENTS agents"
AGENT_PIDS=()
for ((i=0; i<N_AGENTS; i++)); do
  stagger=$(awk "BEGIN{printf \"%.3f\", $i * $STAGGER_STEP}")
  "$PYTHON" session_chaos.py \
    --agent-id "$i" \
    --truth "truth.${i}.log" \
    --stagger "$stagger" \
    "${extra_args[@]}" &
  AGENT_PIDS+=($!)
done

for pid in "${AGENT_PIDS[@]}"; do
  wait "$pid" || true
done

sleep 0.3
TRACE_LINES=$(wc -l <trace.txt | tr -d ' ')
TRUTH_TOTAL=0
for ((i=0; i<N_AGENTS; i++)); do
  n=$(grep -vc '^#' "truth.${i}.log" 2>/dev/null || echo 0)
  TRUTH_TOTAL=$((TRUTH_TOTAL + n))
done
echo "[chaos] trace.txt: $TRACE_LINES lines, $N_AGENTS truth logs, $TRUTH_TOTAL total injection records"
echo "[chaos] check: bash check_violations.sh    # injected vs caught"
echo "               bash individual_violations.sh  # cross-agent isolation"
