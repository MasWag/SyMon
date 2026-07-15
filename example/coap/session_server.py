"""OSCORE-shape session server: instrumented aiocoap server emitting a
mixed wire-layer + OSCORE-layer trace for SyMon monitoring.

Two layers of events in one trace:
  - Wire layer: send_CON, send_NON, recv_ACK captured from instance
    hooks on aiocoap's MessageManager (dispatch_message and
    _send_via_transport). send_req, send_resp captured inside
    HelloResource.render_get. The MessageManager hooks sit BELOW
    aiocoap's dedup layer, so retransmits and MID reuse appear on the
    wire faithfully.
  - OSCORE-shape layer: session_start, session_renew, oscore_msg,
    driven by URI query parameters (kid, ssn, action, old_kid) that
    the client (driver or chaos) sets. The server just faithfully
    records what the client claims — a trace-layer simulation, not
    real OSCORE cryptography. Good enough for monitor development.

Trace-layer injection: ?inject=bad_token asks the server to lie about
the response token, letting a client trigger a §5.3.1 violation
without restarting the server. Same trade-off applies: it's a lie in
the trace, not on the wire.

Predicate signatures (field names match the .symon canon):
  send_CON       src dest mid                  # 2 strings, 1 number
  send_NON       src dest mid                  # 2 strings, 1 number
  recv_ACK       src dest mid                  # 2 strings, 1 number
  send_req       src dest token mid            # 3 strings, 1 number
  send_resp      src dest token mid status     # 3 strings, 2 numbers
  session_start  client server kid             # 3 strings
  session_renew  client server old_kid new_kid # 4 strings
  oscore_msg     client server kid ssn         # 3 strings, 1 number
"""

import argparse
import asyncio
import time
import aiocoap
import aiocoap.resource as resource
from aiocoap.numbers.types import Type
from aiocoap.messagemanager import MessageManager

SERVER_ADDR = "127.0.0.1:5683"


class TraceEmitter:
    """Owns the trace file and the run-start clock. Passed into the
    resource and the MessageManager hooks so no module-level state
    escapes main()."""

    def __init__(self, trace_fh, server_addr):
        self.fh = trace_fh
        self.start = time.monotonic()
        self.server_addr = server_addr

    def emit(self, predicate, *strings, nums=()):
        if isinstance(nums, int):
            nums = (nums,)
        t = time.monotonic() - self.start
        line = "\t".join(
            [predicate, *strings, *(str(n) for n in nums), f"{t:.6f}"]
        )
        self.fh.write(line + "\n")
        self.fh.flush()


def _parse_query(uri_query):
    out = {}
    for q in uri_query:
        if isinstance(q, bytes):
            q = q.decode()
        if "=" in q:
            k, v = q.split("=", 1)
            out[k] = v
    return out


def install_message_layer_hooks(ctx, emitter):
    for ri in ctx.request_interfaces:
        ti = getattr(ri, "token_interface", None)
        if isinstance(ti, MessageManager):
            _wrap(ti, emitter)


def _wrap(mman, emitter):
    original_dispatch = mman.dispatch_message
    original_send = mman._send_via_transport

    def traced_dispatch(message):
        if message.mtype is Type.CON:
            emitter.emit("send_CON", message.remote.hostinfo,
                         emitter.server_addr, nums=message.mid)
        elif message.mtype is Type.NON:
            emitter.emit("send_NON", message.remote.hostinfo,
                         emitter.server_addr, nums=message.mid)
        return original_dispatch(message)

    def traced_send(message):
        if message.mtype is Type.ACK:
            emitter.emit("recv_ACK", emitter.server_addr,
                         message.remote.hostinfo, nums=message.mid)
        return original_send(message)

    mman.dispatch_message = traced_dispatch
    mman._send_via_transport = traced_send


class HelloResource(resource.Resource):
    def __init__(self, emitter):
        super().__init__()
        self.emitter = emitter

    async def render_get(self, request):
        emit = self.emitter.emit
        server_addr = self.emitter.server_addr

        client_addr = request.remote.hostinfo
        token_hex = request.token.hex() or "00"
        mid = request.mid
        q = _parse_query(request.opt.uri_query)

        emit("send_req", client_addr, server_addr, token_hex, nums=mid)

        kid = q.get("kid", "00")
        ssn = int(q.get("ssn", "0"))
        action = q.get("action", "msg")

        if action == "start":
            emit("session_start", client_addr, server_addr, kid)
        elif action == "renew":
            old_kid = q.get("old_kid", "??")
            emit("session_renew", client_addr, server_addr, old_kid, kid)

        emit("oscore_msg", client_addr, server_addr, kid, nums=ssn)

        response = aiocoap.Message(payload=b"ok", code=aiocoap.CONTENT)
        status = response.code.value if hasattr(response.code, "value") else int(response.code)
        # Per-request trace-layer injection: URI-query-driven so
        # multi-agent chaos can mix clean and lying responses without
        # restarting the server.
        resp_token = "BAD!" if q.get("inject") == "bad_token" else token_hex
        emit("send_resp", server_addr, client_addr, resp_token, nums=(mid, status))
        return response


async def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--trace", default="trace.txt")
    args = parser.parse_args()

    with open(args.trace, "w") as trace_fh:
        emitter = TraceEmitter(trace_fh, SERVER_ADDR)
        root = resource.Site()
        root.add_resource(["hello"], HelloResource(emitter))
        ctx = await aiocoap.Context.create_server_context(
            root, bind=("127.0.0.1", 5683)
        )
        install_message_layer_hooks(ctx, emitter)
        print(f"[session_server] writing to {args.trace}")
        try:
            await asyncio.get_event_loop().create_future()
        except asyncio.CancelledError:
            pass
        finally:
            await ctx.shutdown()


if __name__ == "__main__":
    asyncio.run(main())
