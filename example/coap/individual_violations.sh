#!/usr/bin/env bash
# individual_violations.sh — cross-agent isolation proof. Show which
# agent injected which violation classes, then find ANY (agent, monitor)
# pair where the agent did NOT inject the violation that monitor
# catches, and prove zero matches for that agent's KIDs in that
# monitor's output. Run after run_chaos.sh.
set -eo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

echo "=== which agents injected which? ==="
for f in truth.*.log; do
  a=$(basename "$f" .log | sed 's/truth\.//')
  printf "a%s: " "$a"
  grep -hvE '^#|^$' "$f" | awk -F'\t' '{print $3}' | sort | uniq -c | tr '\n' '|'
  echo
done

watches_for() {
  case "$1" in
    token_echo)    echo "bad_token" ;;
    mid_reuse)     echo "mid_reuse" ;;
    session_order) echo "stale_kid" ;;
    session_ssn)   echo "ssn_replay loss_no_renew" ;;
  esac
}

target_agent=""
target_monitor=""
for f in truth.*.log; do
  a=$(basename "$f" .log | sed 's/truth\.//')
  injected=$(grep -hvE '^#|^$' "$f" | awk -F'\t' '{print $3}' | sort -u)
  for m in token_echo mid_reuse session_order session_ssn; do
    saw=0
    for kind in $(watches_for "$m"); do
      if echo "$injected" | grep -qx "$kind"; then saw=1; break; fi
    done
    if [[ $saw -eq 0 ]]; then
      target_agent="$a"
      target_monitor="$m"
      break 2
    fi
  done
done

if [[ -z "$target_agent" ]]; then
  echo
  echo "(every agent injected every monitored class this run; rerun act4)"
  exit 0
fi

echo
echo "=== isolation proof ==="
echo "agent ${target_agent} did NOT inject anything ${target_monitor} catches"
echo "expected: 0 ${target_monitor} matches with c${target_agent}_ prefix"
n=$(symon -nf "${target_monitor}.symon" < trace.txt 2>/dev/null | grep -c "c${target_agent}_" || true)
echo "actual:   ${n}"
