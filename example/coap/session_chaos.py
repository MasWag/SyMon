"""session_chaos.py — randomized multi-agent chaos generator.

One process per agent, driven by --agent-id. Each iteration either:
  - exercises a clean session (establish + a few monotonic messages), or
  - injects one of five violations, logged to truth.log for later
    comparison against the SyMon monitors.

All injections are client-side: the server faithfully records whatever
URI query the client sends (see session_server.py), so violations
manifest in the trace as the client-shaped events the SyMon specs
expect.

Injections:
  ssn_replay     — replay an earlier SSN within an open session
                   (RFC 8613 §3.2.2; session_ssn.symon fires)
  loss_no_renew  — restart SSN at 0 without session_renew, same KID
                   (§7.5 loss-of-mutable-state; session_ssn.symon fires)
  stale_kid      — renew kid_a -> kid_b, then send under kid_a
                   (stale Security Context use; session_order.symon fires)
  mid_reuse      — two send_CON with the same (src, dst, mid) after
                   the first exchange's ACK (RFC 7252 §4.5;
                   mid_reuse.symon fires)
  bad_token      — server lies about the response token
                   (RFC 7252 §5.3.1; token_echo.symon fires)
"""

import argparse
import asyncio
import random
import secrets
import sys
import time

import aiocoap
from aiocoap.messagemanager import MessageManager

URI = "coap://127.0.0.1/hello"

# Defaults for CLI-configurable tunables. Overridable via
# --rate / --violation-prob / --duration / --clean-burst on the
# command line; the actual runtime values end up in every truth
# log's header line so downstream analysis can trust them.
DEFAULT_RATE = 1.0
DEFAULT_VIOLATION_PROB = 0.4
DEFAULT_DURATION = 15.0
DEFAULT_CLEAN_BURST = 3

MID_BURST_COUNT = 2
MID_RANGE = 10000

INJECTION_WEIGHTS = {
    "ssn_replay":    1.0,
    "loss_no_renew": 1.0,
    "stale_kid":     1.0,
    "mid_reuse":     1.0,
    "bad_token":     1.0,
}


def new_kid(agent_id):
    return f"c{agent_id}_{secrets.token_hex(3)}"


async def _send(ctx, kid, ssn, action=None, old_kid=None, timeout=2.0):
    query = [f"kid={kid}", f"ssn={ssn}"]
    if action is not None:
        query.append(f"action={action}")
    if old_kid is not None:
        query.append(f"old_kid={old_kid}")
    msg = aiocoap.Message(code=aiocoap.GET, uri=f"{URI}?{'&'.join(query)}")
    try:
        await asyncio.wait_for(ctx.request(msg).response, timeout=timeout)
    except Exception as e:
        print(f"[chaos]   send failed ({type(e).__name__}): {e}", file=sys.stderr)


class Chaos:
    def __init__(self, truth_fh, agent_id, rate, violation_prob, clean_burst):
        self.truth = truth_fh
        self.agent_id = agent_id
        self.rate = rate
        self.violation_prob = violation_prob
        self.clean_burst = clean_burst
        self.start = time.monotonic()
        self._mid_base = 1000 + agent_id * MID_RANGE
        self._mid = self._mid_base

    def alloc_mid(self):
        self._mid += 1
        if self._mid >= self._mid_base + MID_RANGE:
            self._mid = self._mid_base + 1
        return self._mid

    def t(self):
        return time.monotonic() - self.start

    def log(self, kind, **fields):
        bits = "\t".join(f"{k}={v}" for k, v in fields.items())
        self.truth.write(f"{self.t():.6f}\tagent={self.agent_id}\t{kind}\t{bits}\n")
        self.truth.flush()

    def pick_injection(self):
        names = list(INJECTION_WEIGHTS.keys())
        weights = [INJECTION_WEIGHTS[n] for n in names]
        return random.choices(names, weights=weights, k=1)[0]

    async def clean(self, ctx):
        kid = new_kid(self.agent_id)
        await _send(ctx, kid, 0, action="start")
        for i in range(1, self.clean_burst):
            await asyncio.sleep(0.1)
            await _send(ctx, kid, i)

    async def ssn_replay_inject(self, ctx):
        kid = new_kid(self.agent_id)
        self.log("ssn_replay", kid=kid)
        print(f"[chaos a{self.agent_id}] {self.t():7.2f}s  INJECT ssn_replay kid={kid}")
        await _send(ctx, kid, 0, action="start")
        for i in range(1, 4):
            await asyncio.sleep(0.1)
            await _send(ctx, kid, i)
        await asyncio.sleep(0.1)
        await _send(ctx, kid, 2)  # replay

    async def loss_no_renew_inject(self, ctx):
        kid = new_kid(self.agent_id)
        self.log("loss_no_renew", kid=kid)
        print(f"[chaos a{self.agent_id}] {self.t():7.2f}s  INJECT loss_no_renew kid={kid}")
        await _send(ctx, kid, 0, action="start")
        for i in range(1, 4):
            await asyncio.sleep(0.1)
            await _send(ctx, kid, i)
        await asyncio.sleep(0.1)
        await _send(ctx, kid, 0)  # restart, same kid, no renew

    async def stale_kid_inject(self, ctx):
        kid_a, kid_b = new_kid(self.agent_id), new_kid(self.agent_id)
        self.log("stale_kid", old_kid=kid_a, new_kid=kid_b)
        print(f"[chaos a{self.agent_id}] {self.t():7.2f}s  INJECT stale_kid {kid_a}->{kid_b}->{kid_a}")
        await _send(ctx, kid_a, 0, action="start")
        await asyncio.sleep(0.1)
        await _send(ctx, kid_a, 1)
        await asyncio.sleep(0.1)
        await _send(ctx, kid_b, 0, action="renew", old_kid=kid_a)
        await asyncio.sleep(0.1)
        await _send(ctx, kid_b, 1)
        await asyncio.sleep(0.1)
        await _send(ctx, kid_a, 99)  # stale KID use

    async def bad_token_inject(self, ctx):
        """Wire-layer violation: server lies about the response token.
        RFC 7252 §5.3.1. Caught by token_echo.symon. One injection = one
        request flagged with ?inject=bad_token; one expected match."""
        kid = new_kid(self.agent_id)
        self.log("bad_token", kid=kid)
        print(f"[chaos a{self.agent_id}] {self.t():7.2f}s  INJECT bad_token kid={kid}")
        # Use a fresh session so the bad_token request is self-contained
        # (session_start + a single oscore_msg, no SSN-class confusion).
        await _send(ctx, kid, 0, action="start")
        await asyncio.sleep(0.1)
        # Pass inject=bad_token alongside the regular session params.
        query = f"kid={kid}&ssn=1&inject=bad_token"
        msg = aiocoap.Message(code=aiocoap.GET, uri=f"{URI}?{query}")
        try:
            await asyncio.wait_for(ctx.request(msg).response, timeout=2.0)
        except Exception as e:
            print(f"[chaos a{self.agent_id}]   bad_token send failed: {type(e).__name__}", file=sys.stderr)

    async def mid_reuse_burst_inject(self, ctx):
        """Wire-layer violation: two send_CON events with the same
        (src, dst, mid) after the first exchange's ACK. RFC 7252 §4.5.
        Caught by mid_reuse.symon.

        aiocoap clears any user-set MID before sending (see
        messagemanager.py's `_next_message_id`), so the escape hatch
        is to spin up a fresh sub-context, monkey-patch its
        MessageManager instance so `_next_message_id` returns a fixed
        MID, send a short burst under that MID, and tear the
        sub-context down. Isolating this to a sub-context keeps the
        agent's main context clean for other injections."""
        mid = self.alloc_mid()
        kid = new_kid(self.agent_id)
        self.log("mid_reuse", kid=kid, mid=mid, count=MID_BURST_COUNT)
        print(f"[chaos a{self.agent_id}] {self.t():7.2f}s  INJECT mid_reuse kid={kid} mid={mid}")

        burst_ctx = await aiocoap.Context.create_client_context()
        try:
            for ri in burst_ctx.request_interfaces:
                ti = getattr(ri, "token_interface", None)
                if isinstance(ti, MessageManager):
                    ti._next_message_id = lambda m=mid: m
            for i in range(MID_BURST_COUNT):
                action = "start" if i == 0 else None
                query = [f"kid={kid}", f"ssn={i}"]
                if action:
                    query.append(f"action={action}")
                msg = aiocoap.Message(code=aiocoap.GET, uri=f"{URI}?{'&'.join(query)}")
                try:
                    await asyncio.wait_for(burst_ctx.request(msg).response, timeout=0.5)
                except asyncio.TimeoutError:
                    pass
                await asyncio.sleep(0.3)
        finally:
            await burst_ctx.shutdown()

    INJECTORS = {
        "ssn_replay":    ssn_replay_inject,
        "loss_no_renew": loss_no_renew_inject,
        "stale_kid":     stale_kid_inject,
        "mid_reuse":     mid_reuse_burst_inject,
        "bad_token":     bad_token_inject,
    }

    async def loop(self, duration):
        ctx = await aiocoap.Context.create_client_context()
        end = self.start + duration if duration > 0 else float("inf")
        gap = 1.0 / self.rate
        try:
            while time.monotonic() < end:
                if random.random() < self.violation_prob:
                    name = self.pick_injection()
                    await self.INJECTORS[name](self, ctx)
                else:
                    await self.clean(ctx)
                await asyncio.sleep(gap)
        finally:
            await ctx.shutdown()


async def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--agent-id", type=int, default=0)
    parser.add_argument("--truth", default="truth.log")
    parser.add_argument("--duration", type=float, default=DEFAULT_DURATION,
                        help="Seconds to run before stopping.")
    parser.add_argument("--rate", type=float, default=DEFAULT_RATE,
                        help="Iterations per second (gap = 1/rate).")
    parser.add_argument("--violation-prob", type=float,
                        default=DEFAULT_VIOLATION_PROB,
                        help="Per-iteration probability of injecting a violation.")
    parser.add_argument("--clean-burst", type=int,
                        default=DEFAULT_CLEAN_BURST,
                        help="Messages per clean session (SSN 0..clean_burst-1).")
    parser.add_argument("--stagger", type=float, default=0.0)
    args = parser.parse_args()

    if args.stagger > 0:
        await asyncio.sleep(args.stagger)

    with open(args.truth, "w") as truth_fh:
        truth_fh.write(
            f"# session_chaos agent={args.agent_id} RATE={args.rate} "
            f"VIOLATION_PROB={args.violation_prob} DURATION={args.duration} "
            f"CLEAN_BURST={args.clean_burst} STAGGER={args.stagger}\n"
            f"# weights: {INJECTION_WEIGHTS}\n"
        )
        truth_fh.flush()
        chaos = Chaos(
            truth_fh,
            args.agent_id,
            rate=args.rate,
            violation_prob=args.violation_prob,
            clean_burst=args.clean_burst,
        )
        print(
            f"[chaos a{args.agent_id}] starting "
            f"(rate={args.rate} violation_prob={args.violation_prob} "
            f"duration={args.duration}s)"
        )
        try:
            await chaos.loop(args.duration)
        except KeyboardInterrupt:
            print(f"[chaos a{args.agent_id}] interrupted at {chaos.t():.2f}s")
    print(f"[chaos a{args.agent_id}] done; truth log: {args.truth}")


if __name__ == "__main__":
    asyncio.run(main())
