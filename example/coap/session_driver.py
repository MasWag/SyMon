"""Manual scenario driver for session_server.py: pick one OSCORE-shape
scenario, exercise the server end-to-end, exit. Each scenario leaves
a deterministic trace segment SyMon can be run against. Use this for
per-property demos; session_chaos.py is the randomized multi-agent
generator."""

import argparse
import asyncio
import secrets
import aiocoap
from aiocoap.messagemanager import MessageManager

MID_REUSE_FIXED = 12345

URI = "coap://127.0.0.1/hello"


def new_kid():
    return secrets.token_hex(4)


async def _send(ctx, kid, ssn, action=None, old_kid=None):
    query = [f"kid={kid}", f"ssn={ssn}"]
    if action is not None:
        query.append(f"action={action}")
    if old_kid is not None:
        query.append(f"old_kid={old_kid}")
    msg = aiocoap.Message(code=aiocoap.GET, uri=f"{URI}?{'&'.join(query)}")
    await ctx.request(msg).response


async def drive_clean(ctx, count, gap):
    """Establish a session and send `count` messages with strictly
    increasing SSN. No renewal. Monitor stays silent."""
    kid = new_kid()
    await _send(ctx, kid, 0, action="start")
    for i in range(1, count):
        await asyncio.sleep(gap)
        await _send(ctx, kid, i)


async def drive_renew(ctx, count, gap):
    """Establish on kid_a, send half a session, renew to kid_b, send the
    rest. Legal: renewal flips the kid and resets SSN. Monitor stays
    silent because the saved (kid_a) state and the new (kid_b) stream
    don't overlap on kid."""
    half = max(1, count // 2)
    kid_a = new_kid()
    await _send(ctx, kid_a, 0, action="start")
    for i in range(1, half):
        await asyncio.sleep(gap)
        await _send(ctx, kid_a, i)

    await asyncio.sleep(gap)
    kid_b = new_kid()
    await _send(ctx, kid_b, 0, action="renew", old_kid=kid_a)
    for i in range(1, max(1, count - half)):
        await asyncio.sleep(gap)
        await _send(ctx, kid_b, i)


async def drive_ssn_replay(ctx, count, gap):
    """Send 0..count-1, then send SSN=2 again — same KID, non-increasing.
    Violates RFC 8613 §3.2.2 SSN monotonicity. Monitor fires."""
    kid = new_kid()
    await _send(ctx, kid, 0, action="start")
    for i in range(1, count):
        await asyncio.sleep(gap)
        await _send(ctx, kid, i)
    await asyncio.sleep(gap)
    await _send(ctx, kid, 2)


async def drive_loss_no_renew(ctx, count, gap):
    """Send 0..count-1, then restart at SSN=0 with the SAME KID — no
    session_renew event. Models RFC 8613 §7.5 "loss of mutable Security
    Context" where the sender reboots and resumes without rekeying.
    Monitor fires; this is the replay-vulnerability footgun §7.5 names."""
    kid = new_kid()
    await _send(ctx, kid, 0, action="start")
    for i in range(1, count):
        await asyncio.sleep(gap)
        await _send(ctx, kid, i)
    await asyncio.sleep(gap)
    await _send(ctx, kid, 0)


async def drive_bad_token(ctx, count, gap):
    """Establish a session, then send one request tagged
    ?inject=bad_token so the server echoes a wrong token in the
    response. Violates RFC 7252 §5.3.1; token_echo.symon fires.
    `count` is used to precede the injection with `count - 1` clean
    messages so the flagged response is visibly the odd one out."""
    kid = new_kid()
    await _send(ctx, kid, 0, action="start")
    for i in range(1, max(1, count - 1)):
        await asyncio.sleep(gap)
        await _send(ctx, kid, i)
    await asyncio.sleep(gap)
    msg = aiocoap.Message(
        code=aiocoap.GET,
        uri=f"{URI}?kid={kid}&ssn={max(1, count - 1)}&inject=bad_token",
    )
    await ctx.request(msg).response


async def drive_mid_reuse(ctx, count, gap):
    """Send `count` requests all under the same fixed Message ID.
    Violates RFC 7252 §4.5 MID non-reuse; mid_reuse.symon fires.

    aiocoap clears any user-set MID before sending (messagemanager.py
    _next_message_id), so the escape hatch is to spin up a fresh
    sub-context and monkey-patch its MessageManager instance's
    _next_message_id to return a fixed value. Uses per-request
    wait_for because after the first exchange the server replays
    cached responses (dedup layer) and the client's token correlator
    can't match them — subsequent requests hang until timeout, which
    is expected here.

    The passed-in `ctx` is unused (a fresh sub-context is required to
    isolate the monkey-patch); keeping the signature uniform with the
    other scenarios."""
    del ctx
    kid = new_kid()
    burst_ctx = await aiocoap.Context.create_client_context()
    try:
        for ri in burst_ctx.request_interfaces:
            ti = getattr(ri, "token_interface", None)
            if isinstance(ti, MessageManager):
                ti._next_message_id = lambda m=MID_REUSE_FIXED: m
        for i in range(count):
            query = [f"kid={kid}", f"ssn={i}"]
            if i == 0:
                query.append("action=start")
            msg = aiocoap.Message(
                code=aiocoap.GET, uri=f"{URI}?{'&'.join(query)}"
            )
            try:
                await asyncio.wait_for(
                    burst_ctx.request(msg).response, timeout=gap
                )
            except asyncio.TimeoutError:
                pass
            await asyncio.sleep(gap)
    finally:
        await burst_ctx.shutdown()


async def drive_stale_kid(ctx, count, gap):
    """Establish on kid_a, renew to kid_b, then send another message
    with kid_a — the rotated-out KID. Violates session_order: post-renewal
    use of an old Security Context. Monitor (session_order.symon) fires."""
    kid_a = new_kid()
    await _send(ctx, kid_a, 0, action="start")
    for i in range(1, max(1, count // 2)):
        await asyncio.sleep(gap)
        await _send(ctx, kid_a, i)

    await asyncio.sleep(gap)
    kid_b = new_kid()
    await _send(ctx, kid_b, 0, action="renew", old_kid=kid_a)
    for i in range(1, max(1, count - count // 2)):
        await asyncio.sleep(gap)
        await _send(ctx, kid_b, i)

    # The violation: a stray oscore_msg on the rotated-out kid_a.
    await asyncio.sleep(gap)
    await _send(ctx, kid_a, 99)


SCENARIOS = {
    "clean":         drive_clean,
    "renew":         drive_renew,
    "ssn_replay":    drive_ssn_replay,
    "loss_no_renew": drive_loss_no_renew,
    "stale_kid":     drive_stale_kid,
    "bad_token":     drive_bad_token,
    "mid_reuse":     drive_mid_reuse,
}


async def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("scenario", choices=SCENARIOS.keys())
    parser.add_argument("--count", type=int, default=5,
                        help="Number of messages per session segment.")
    parser.add_argument("--gap", type=float, default=0.2,
                        help="Seconds to sleep between requests.")
    args = parser.parse_args()

    ctx = await aiocoap.Context.create_client_context()
    try:
        await SCENARIOS[args.scenario](ctx, args.count, args.gap)
    finally:
        await ctx.shutdown()


if __name__ == "__main__":
    asyncio.run(main())
