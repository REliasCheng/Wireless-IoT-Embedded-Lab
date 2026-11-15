"""Scripted MQTT 3.1.1 peer on loopback. No Mosquitto broker configuration is implied."""

import json
from pathlib import Path
import socket
import subprocess
import sys


TOPIC = b"lab/suback/sensor1/command"


def check(condition, message):
    if not condition:
        raise AssertionError(message)


def read_exact(peer, size):
    data = bytearray()
    while len(data) < size:
        chunk = peer.recv(size - len(data))
        if not chunk:
            raise EOFError("MQTT connection closed mid-read")
        data.extend(chunk)
    return bytes(data)


def packet(peer):
    kind = read_exact(peer, 1)[0]
    size = 0
    for shift in range(0, 28, 7):
        byte = read_exact(peer, 1)[0]
        size |= (byte & 127) << shift
        if byte < 128:
            check(size <= 4096, "unexpected test packet length")
            return kind, read_exact(peer, size)
    raise AssertionError("invalid MQTT packet length")


def send(peer, kind, body=b""):
    check(len(body) < 128, "test fixture expects short packets")
    peer.sendall(bytes((kind, len(body))) + body)


def command(peer, request_id):
    body = json.dumps({"v": 1, "request_id": request_id, "command": "set_switch", "value": 1})
    send(peer, 0x30, len(TOPIC).to_bytes(2, "big") + TOPIC + body.encode("ascii"))


def subscribe(peer):
    peer.settimeout(4)
    kind, body = packet(peer)
    check(kind == 0x10 and body[:7] == b"\0\4MQTT\4", "expected MQTT 3.1.1 CONNECT")
    send(peer, 0x20, b"\0\0")
    kind, body = packet(peer)
    expected = len(TOPIC).to_bytes(2, "big") + TOPIC + b"\1"
    check(kind == 0x82 and body[2:] == expected, "expected command SUBSCRIBE with QoS 1")
    return body[:2]


def validate(device):
    with socket.socket() as listener:
        listener.bind(("127.0.0.1", 0))
        listener.listen(2)
        listener.settimeout(5)
        proc = subprocess.Popen(
            [device, "--port", str(listener.getsockname()[1]), "--run-id", "suback",
             "--device-id", "sensor1", "--qos", "1", "--duration-ms", "4000"],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
        )
        try:
            first, _ = listener.accept()
            with first:
                mid = subscribe(first)
                command(first, 7)
                send(first, 0x90, mid + b"\0")
                try:
                    kind, body = packet(first)
                    check(kind == 0xE0 and not body, f"expected DISCONNECT, observed 0x{kind:02x}")
                except EOFError:
                    pass  # Some libmosquitto versions close without flushing DISCONNECT.
            second, _ = listener.accept()
            with second:
                mid = subscribe(second)
                send(second, 0x90, mid + b"\1")
                command(second, 8)
                observed = set()
                while observed != {"ack", "telemetry"}:
                    kind, body = packet(second)
                    check(kind == 0x32, f"expected QoS 1 PUBLISH, observed 0x{kind:02x}")
                    size = int.from_bytes(body[:2], "big")
                    topic = body[2:2+size].decode("ascii")
                    payload = json.loads(body[4+size:])
                    send(second, 0x40, body[2+size:4+size])
                    if topic == "lab/suback/sensor1/ack":
                        check(payload["request_id"] == 8 and payload["accepted"], str(payload))
                        observed.add("ack")
                    else:
                        check(topic == "lab/suback/sensor1/telemetry", topic)
                        observed.add("telemetry")
                out, _ = proc.communicate(timeout=6)
            check(proc.returncode == 0, f"device returned {proc.returncode}")
            markers = ["STATE CONNECTED", "STATE BACKOFF", "reason=suback_failed",
                       "STATE RECONNECTING", "STATE CONNECTED", "STATE SUBSCRIBED",
                       "STATE RUNNING", "EVENT COMMAND request_id=8 result=accepted"]
            cursor = 0
            for marker in markers:
                pos = out.find(marker, cursor)
                check(pos >= 0, f"missing ordered marker {marker!r}")
                cursor = pos + len(marker)
            check(out.count("STATE RUNNING") == 1, "entered RUNNING on a downgraded subscription")
            check("request_id=7" not in out and "commands=1 rejected=0" in out,
                  "business command ran before successful subscription")
        finally:
            if proc.poll() is None:
                proc.kill()
            out, _ = proc.communicate(timeout=3)
            log_dir = Path("artifacts/suback_downgrade")
            log_dir.mkdir(parents=True, exist_ok=True)
            (log_dir / "device.log").write_text(out, encoding="utf-8")
            print(out, end="")


if __name__ == "__main__":
    validate(sys.argv[1])
