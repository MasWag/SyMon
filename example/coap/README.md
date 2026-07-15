# CoAP + OSCORE session monitoring with SyMon

Runtime monitoring of a CoAP / OSCORE-shape server against RFC 7252
(CoAP) and RFC 8613 (OSCORE). A chaos generator injects violations,
an instrumented aiocoap server captures a mixed wire-layer +
OSCORE-layer trace, and SyMon specs check the trace against six spec
properties.

## Properties monitored

| Spec file | Property | RFC |
|---|---|---|
| `con_ack.symon` | Every CON is followed by a matching ACK within the ACK window. | RFC 7252 §4.4, §5.2.2 |
| `mid_reuse.symon` | No two CONs with the same `(src, dest, mid)` after a completed exchange within `EXCHANGE_LIFETIME`. | RFC 7252 §4.5 |
| `retransmit_count.symon` | At most `MAX_RETRANSMIT = 4` retransmissions of the same CON. | RFC 7252 §4.8 |
| `token_echo.symon` | Every response echoes its request's token and uses the flipped endpoints. | RFC 7252 §5.3.1 |
| `session_ssn.symon` | SSN strictly increases within an OSCORE Security Context. Also catches loss-of-mutable-state, wire-indistinguishable from a replay. | RFC 8613 §3.2.2, §7.2.1, §7.5 |
| `session_order.symon` | The rotated-out KID is not used after `session_renew`. | RFC 8613 App. B |

## Trace format

Tab-separated: predicate, then string args, then number args, then a
timestamp in seconds since server start. Every `.symon` file shares
an identical 8-signature block covering all events emitted by
`session_server.py`.

| Predicate       | Strings                       | Numbers      |
|-----------------|-------------------------------|--------------|
| `send_CON`      | src, dest                     | mid          |
| `send_NON`      | src, dest                     | mid          |
| `recv_ACK`      | src, dest                     | mid          |
| `send_req`      | src, dest, token              | mid          |
| `send_resp`     | src, dest, token              | mid, status  |
| `session_start` | client, server, kid           | —            |
| `session_renew` | client, server, old_kid, new_kid | —         |
| `oscore_msg`    | client, server, kid           | ssn          |

## Setup

```sh
python3 -m venv .venv
./.venv/bin/pip install aiocoap
```

The shell runners autodetect `./.venv/` (falling back to `../.venv/`,
then system `python3`).

## Running

Single scenario end-to-end:

```sh
bash run.sh SCENARIO
# SCENARIO ∈ {clean, renew, ssn_replay, loss_no_renew,
#            stale_kid, bad_token, mid_reuse}
```

Multi-agent chaos, then post-hoc analysis:

```sh
bash run_chaos.sh                 # writes trace.txt + truth.*.log
bash check_violations.sh          # injected vs caught
```

Chaos tunables via env vars: `N_AGENTS`, `STAGGER_STEP`, `DURATION`,
`RATE`, `VIOLATION_PROB`, `CLEAN_BURST`.

Direct monitor invocation:

```sh
symon -nf mid_reuse.symon      < trace.txt
symon -nf token_echo.symon     < trace.txt
symon -nf session_ssn.symon    < trace.txt
symon -nf session_order.symon  < trace.txt
symon -nf retransmit_count.symon < trace.txt
./con_ack.symon                < trace.txt   # parametric mode via shebang
```
