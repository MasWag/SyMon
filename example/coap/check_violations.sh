#!/usr/bin/env bash
# check_violations.sh — show what got injected, then show what the
# monitors caught. Run after run_chaos.sh has produced trace.txt and
# truth.*.log.
set -eo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

echo "=== injection breakdown (truth) ==="
grep -hvE '^#|^$' truth.*.log | awk -F'\t' '{print $3}' | sort | uniq -c

echo
echo "=== monitor matches ==="
for m in mid_reuse token_echo session_ssn session_order; do
  out=$(symon -nf "${m}.symon" < trace.txt 2>&1)
  matches=$(echo "$out" | grep -cE '^@')
  warns=$(echo "$out" | grep -c '^Undefined' || true)
  printf "%-15s matches=%s  warnings=%s\n" "$m" "$matches" "$warns"
done

echo
echo "=== expected vs actual (session_ssn) ==="
# session_ssn.symon catches both ssn_replay AND loss_no_renew: RFC 8613
# §7.5 treats loss-of-mutable-state as a spec violation, indistinguish-
# able on the wire from a replay (same tuple, non-increasing SSN).
# Each real violation now produces exactly one match.
lnr=$(grep -hvE '^#|^$' truth.*.log | awk -F'\t' '$3 == "loss_no_renew"' | wc -l | tr -d ' ')
sr=$(grep -hvE '^#|^$' truth.*.log | awk -F'\t' '$3 == "ssn_replay"' | wc -l | tr -d ' ')
ssn_matches=$(symon -nf session_ssn.symon < trace.txt 2>/dev/null | grep -cE '^@')
echo "  loss_no_renew (${lnr}) + ssn_replay (${sr}) = $((lnr + sr))  vs  session_ssn matches: ${ssn_matches}"
