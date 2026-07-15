#!/usr/bin/env bash
# One-scenario end-to-end: server + driver + trace.txt.

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

SERVER_LOG=/tmp/session_server.log
PORT_WAIT=1.0
COUNT=5
GAP=0.2

usage() {
  cat <<EOF
Usage: $0 SCENARIO

Scenarios:
  clean          monotonic SSN, single session                 (silent)
  renew          establish, send, session_renew, send          (silent)
  ssn_replay     monotonic then replay SSN=2 (same KID)        (session_ssn fires)
  loss_no_renew  monotonic then restart SSN=0 (same KID)       (session_ssn fires)
  stale_kid      renew kid_a -> kid_b, then send under kid_a   (session_order fires)
  bad_token      server echoes wrong token in response         (token_echo fires)
  mid_reuse      two send_CON with same (src, dst, mid)        (mid_reuse fires)

Defaults: count=$COUNT requests, gap=${GAP}s.
Output:   trace.txt in $SCRIPT_DIR.

After running, run whichever monitor(s) match the scenario:
  symon -nf session_ssn.symon    < trace.txt
  symon -nf session_order.symon  < trace.txt
  symon -nf token_echo.symon     < trace.txt
  symon -nf mid_reuse.symon      < trace.txt
  ./con_ack.symon                < trace.txt   # parametric mode
EOF
}

# Parse args first so --help works even without Python deps.
if [[ $# -ne 1 ]]; then usage; exit 1; fi
case "$1" in
  -h|--help) usage; exit 0 ;;
  clean|renew|ssn_replay|loss_no_renew|stale_kid|bad_token|mid_reuse) ;;
  *) echo "Unknown scenario: $1" >&2; echo; usage; exit 1 ;;
esac
SCENARIO="$1"

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

existing=$(lsof -ti :5683 2>/dev/null || true)
if [[ -n "$existing" ]]; then
  echo "[run] killing leftover :5683 (PID $existing)"
  kill $existing 2>/dev/null || true
  sleep 0.3
fi

echo "[run] scenario=$SCENARIO count=$COUNT gap=$GAP"
"$PYTHON" session_server.py --trace trace.txt >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!
trap "kill $SERVER_PID 2>/dev/null || true" EXIT

sleep "$PORT_WAIT"

"$PYTHON" session_driver.py "$SCENARIO" --count "$COUNT" --gap "$GAP" \
  || echo "[run] driver exited non-zero"

sleep 0.2
kill "$SERVER_PID" 2>/dev/null || true
wait "$SERVER_PID" 2>/dev/null || true

LINES=$(wc -l <trace.txt | tr -d ' ')
echo "[run] trace.txt: $LINES lines"
case "$SCENARIO" in
  ssn_replay|loss_no_renew)
    echo "[run] check: symon -nf session_ssn.symon    < trace.txt" ;;
  stale_kid)
    echo "[run] check: symon -nf session_order.symon  < trace.txt" ;;
  bad_token)
    echo "[run] check: symon -nf token_echo.symon     < trace.txt" ;;
  mid_reuse)
    echo "[run] check: symon -nf mid_reuse.symon      < trace.txt" ;;
  clean|renew)
    echo "[run] check: no violation expected; every monitor should stay silent" ;;
esac
