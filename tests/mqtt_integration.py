"""Bounded loopback-only integration against separate Mosquitto processes."""

import argparse
import json
import os
from pathlib import Path
import signal
import socket
import subprocess
import sys
import time
import uuid


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def text(path):
    return path.read_text(encoding="utf-8", errors="replace") if path.exists() else ""


def until(predicate, timeout, message):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.04)
    raise RuntimeError(message)


def port_candidate():
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def stop(process):
    if process is None or process.poll() is not None:
        return
    process.terminate()
    try:
        process.wait(timeout=3)
    except subprocess.TimeoutExpired:
        process.kill()  # Only the child created by this harness.
        process.wait(timeout=3)


class Lab:
    def __init__(self, scenario, device):
        self.scenario = scenario
        self.run_id = "r" + uuid.uuid4().hex[:10]
        self.device_id = "sensor1"
        self.root = Path.cwd() / "artifacts" / f"{scenario}-{self.run_id}"
        self.root.mkdir(parents=True, exist_ok=False)
        self.config = self.root / "broker.conf"
        self.broker_log = self.root / "broker.log"
        self.device_log = self.root / "device.log"
        self.port = port_candidate()
        self.device_exe = device
        self.broker = None
        self.client = None
        self.other_client = None
        self._handles = []
        self.prefix = f"lab/{self.run_id}/{self.device_id}"

    def start_broker(self):
        require(self.broker is None or self.broker.poll() is not None, "broker already running")
        self.config.write_text(
            f"listener {self.port} 127.0.0.1\nallow_anonymous true\npersistence false\nlog_dest stdout\n",
            encoding="ascii",
        )
        handle = self.broker_log.open("ab")
        self._handles.append(handle)
        self.broker = subprocess.Popen(
            ["mosquitto", "-c", str(self.config), "-v"],
            stdout=handle, stderr=subprocess.STDOUT,
        )

        def ready():
            if self.broker.poll() is not None:
                raise RuntimeError(f"broker exited rc={self.broker.returncode}: {text(self.broker_log)}")
            with socket.socket() as sock:
                sock.settimeout(0.1)
                return sock.connect_ex(("127.0.0.1", self.port)) == 0

        until(ready, 5, "broker did not bind loopback port")

    def start_device(self, qos=1, max_retries=20, extra=()):
        handle = self.device_log.open("ab")
        self._handles.append(handle)
        self.client = subprocess.Popen(
            [str(self.device_exe), "--port", str(self.port), "--run-id", self.run_id,
             "--device-id", self.device_id, "--qos", str(qos),
             "--duration-ms", "18000", "--max-retries", str(max_retries), *extra],
            stdout=handle, stderr=subprocess.STDOUT,
        )

    def wait_device(self, marker, count=1, timeout=8):
        def seen():
            data = text(self.device_log)
            if self.client.poll() is not None and data.count(marker) < count:
                raise RuntimeError(f"device exited rc={self.client.returncode}: {data}")
            return data.count(marker) >= count

        until(seen, timeout, f"missing device marker {marker!r}: {text(self.device_log)}")

    def subscriber(self, topic, timeout=7):
        tag = uuid.uuid4().hex[:8]
        client_id = "sub_" + tag
        debug = self.root / f"sub-{tag}.log"
        handle = debug.open("wb")
        self._handles.append(handle)
        process = subprocess.Popen(
            ["mosquitto_sub", "-h", "127.0.0.1", "-p", str(self.port),
             "-V", "mqttv311", "-i", client_id, "-t", topic,
             "-q", "1", "-C", "1", "-W", str(timeout), "-F", "%p"],
            stdout=subprocess.PIPE, stderr=handle, text=True,
        )

        def subscribed():
            if process.poll() is not None:
                raise RuntimeError(f"subscriber exited rc={process.returncode}: {text(debug)}")
            return f"Sending SUBACK to {client_id}" in text(self.broker_log)

        until(subscribed, 4,
              f"Broker did not confirm subscriber {client_id}: {text(debug)}; {text(self.broker_log)}")
        return process

    def publish(self, topic, payload, qos=1):
        args = ["mosquitto_pub", "-h", "127.0.0.1", "-p", str(self.port),
                "-V", "mqttv311", "-i", "pub_" + uuid.uuid4().hex[:8],
                "-t", topic, "-q", str(qos)]
        if payload is None:
            args.append("-n")
        else:
            args.extend(["-m", payload])
        result = subprocess.run(args, capture_output=True, text=True, timeout=5)
        require(result.returncode == 0, f"publisher rc={result.returncode}: {result.stderr}")

    def receive(self, process, timeout=8):
        output, _ = process.communicate(timeout=timeout)
        require(process.returncode == 0, f"subscriber rc={process.returncode}: {output}")
        return json.loads(output.strip())

    def command(self, payload, expected, qos=1):
        sub = self.subscriber(self.prefix + "/ack")
        self.publish(self.prefix + "/command", payload, qos)
        ack = self.receive(sub)
        require(ack["v"] == 1 and ack["accepted"] is expected[0] and
                ack["switch"] == expected[1], f"unexpected ACK: {ack}")
        return ack

    def finish(self, passed):
        stop(self.other_client)
        stop(self.client)
        stop(self.broker)
        for handle in self._handles:
            handle.close()
        self.config.unlink(missing_ok=True)
        result = {"scenario": self.scenario, "passed": passed, "run_id": self.run_id,
                  "port": self.port, "device_exit": self.client.returncode if self.client else None,
                  "broker_exit": self.broker.returncode if self.broker else None}
        (self.root / "result.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
        print(f"{self.scenario}: {'PASS' if passed else 'FAIL'} evidence={self.root}")


def normal(lab, qos):
    lab.start_broker()
    telemetry_sub = lab.subscriber(lab.prefix + "/telemetry")
    lab.start_device(qos=qos)
    lab.wait_device("EVENT RUNNING")
    message = lab.receive(telemetry_sub)
    require(message["v"] == 1 and message["sequence"] == 1 and
            message["switch"] == 0 and 0 <= message["humidity_percent"] <= 100,
            f"invalid telemetry: {message}")
    ack = lab.command('{"v":1,"request_id":1,"command":"set_switch","value":1}', (True, 1), qos)
    require(ack["request_id"] == 1 and ack["reason"] == "ok", f"bad ACK: {ack}")
    lab.wait_device("EVENT COMMAND request_id=1 result=accepted")
    if qos == 1:
        lab.wait_device("EVENT PUBLISH_CALLBACK", timeout=5)
    print(f"TRACE telemetry={message} ack={ack}")


def invalid(lab):
    lab.start_broker()
    lab.start_device()
    lab.wait_device("EVENT RUNNING")
    first = lab.command('{"v":1,"request_id":2,"command":"set_switch","value":1}', (True, 1))
    require(first["reason"] == "ok", "valid command failed")
    cases = [
        ("not-json", "invalid"),
        ('{"v":1,"request_id":3,"command":"set_switch"}', "invalid"),
        ('{"v":1,"request_id":3,"command":"toggle","value":0}', "invalid"),
        ('{"v":1,"request_id":3,"command":"set_switch","value":2}', "invalid"),
        (None, "invalid"),
        ("x" * 257, "invalid"),
        ('{"v":1,"request_id":1,"command":"set_switch","value":0}', "stale"),
        ('{"v":1,"request_id":2,"command":"set_switch","value":0}', "conflict"),
    ]
    for payload, reason in cases:
        ack = lab.command(payload, (False, 1))
        require(ack["reason"] == reason, f"wrong rejection: {ack}")
    duplicate = lab.command('{"v":1,"request_id":2,"command":"set_switch","value":1}', (True, 1))
    require(duplicate["duplicate"] is True and duplicate["reason"] == "duplicate", str(duplicate))
    no_ack = lab.subscriber(lab.prefix + "/ack", timeout=1)
    lab.publish(lab.prefix + "/wrong", '{"v":1,"request_id":4,"command":"set_switch","value":0}')
    output, _ = no_ack.communicate(timeout=3)
    require(no_ack.returncode != 0 and not output.strip(), "wrong topic generated ACK")
    final = lab.command('{"v":1,"request_id":3,"command":"set_switch","value":0}', (True, 0))
    require(final["reason"] == "ok", str(final))
    lab.wait_device("result=duplicate")
    print(f"TRACE rejected={len(cases)} duplicate={duplicate} final={final}")


def reconnect(lab):
    lab.start_broker()
    lab.start_device()
    lab.wait_device("EVENT RUNNING")
    stop(lab.broker)
    lab.wait_device("STATE BACKOFF", timeout=5)
    lab.wait_device("EVENT RECONNECT_ATTEMPT", timeout=4)
    lab.wait_device("reason=reconnect_failed", timeout=4)
    lab.start_broker()
    lab.wait_device("EVENT RUNNING", count=2, timeout=8)
    ack = lab.command('{"v":1,"request_id":9,"command":"set_switch","value":1}', (True, 1))
    require(ack["reason"] == "ok", str(ack))
    require("EVENT RECONNECT_ATTEMPT" in text(lab.device_log), "no bounded reconnect attempt")
    print(f"TRACE reconnect={ack}")


def unavailable(lab):
    lab.start_device(max_retries=3)
    lab.client.wait(timeout=8)
    log = text(lab.device_log)
    require(lab.client.returncode != 0 and "EVENT FAILED retry_exhausted" in log,
            f"unavailable Broker not reported: {log}")
    require("EVENT TELEMETRY_QUEUED" not in log and "EVENT RUNNING" not in log,
            "false delivery while Broker unavailable")
    require(log.count("EVENT RECONNECT_ATTEMPT") == 3, "retry limit not enforced")
    print("TRACE unavailable=retry_exhausted attempts=3")


def subscribe_failure(lab):
    lab.start_broker()
    lab.start_device(extra=("--inject-subscribe-fail-once",))
    lab.wait_device("EVENT INJECT_SUBSCRIBE_FAILURE")
    lab.wait_device("EVENT RUNNING", timeout=8)
    require("STATE BACKOFF" in text(lab.device_log) and
            "EVENT RECONNECT_ATTEMPT" in text(lab.device_log), "subscription failure not recovered")
    ack = lab.command('{"v":1,"request_id":11,"command":"set_switch","value":1}', (True, 1))
    print(f"TRACE subscribe_failure_recovered={ack}")


def publish_failure(lab):
    lab.start_broker()
    telemetry_sub = lab.subscriber(lab.prefix + "/telemetry")
    lab.start_device(extra=("--inject-publish-fail-once",))
    lab.wait_device("EVENT INJECT_PUBLISH_FAILURE")
    lab.wait_device("EVENT RUNNING", count=2, timeout=8)
    message = lab.receive(telemetry_sub)
    require(message["sequence"] == 1 and "reason=telemetry_publish_failed" in text(lab.device_log),
            f"failed publish counted as delivery: {message}")
    print(f"TRACE publish_failure_recovered={message}")


def connect_timeout(lab):
    with socket.socket() as silent_peer:
        silent_peer.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        silent_peer.bind(("127.0.0.1", lab.port))
        silent_peer.listen(2)
        lab.start_device(max_retries=2)
        lab.wait_device("reason=connect_timeout", timeout=6)
        log = text(lab.device_log)
        require("EVENT RUNNING" not in log and "EVENT TELEMETRY_QUEUED" not in log,
                "silent TCP peer was misreported as a Broker")
    print("TRACE connect_timeout=detected")


def client_id_collision(lab):
    lab.start_broker()
    lab.start_device()
    lab.wait_device("EVENT RUNNING")
    other_log = lab.root / "collision-device.log"
    handle = other_log.open("wb")
    lab._handles.append(handle)
    lab.other_client = subprocess.Popen(
        [str(lab.device_exe), "--port", str(lab.port), "--run-id", lab.run_id,
         "--device-id", lab.device_id, "--duration-ms", "4000"],
        stdout=handle, stderr=subprocess.STDOUT,
    )
    until(lambda: "EVENT RUNNING" in text(other_log), 5,
          f"collision client did not connect: {text(other_log)}")
    lab.wait_device("STATE BACKOFF", timeout=5)
    stop(lab.other_client)
    lab.wait_device("EVENT RUNNING", count=2, timeout=8)
    ack = lab.command('{"v":1,"request_id":13,"command":"set_switch","value":1}', (True, 1))
    print(f"TRACE client_id_collision_recovered={ack}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--scenario", required=True,
                        choices=["normal_qos0", "normal_qos1", "invalid_command", "reconnect",
                                 "unavailable", "subscribe_failure", "publish_failure",
                                 "connect_timeout", "client_id_collision"])
    parser.add_argument("--device", type=Path, required=True)
    args = parser.parse_args()
    lab = Lab(args.scenario, args.device)
    passed = False
    try:
        if args.scenario == "normal_qos0": normal(lab, 0)
        elif args.scenario == "normal_qos1": normal(lab, 1)
        elif args.scenario == "invalid_command": invalid(lab)
        elif args.scenario == "reconnect": reconnect(lab)
        elif args.scenario == "unavailable": unavailable(lab)
        elif args.scenario == "subscribe_failure": subscribe_failure(lab)
        elif args.scenario == "publish_failure": publish_failure(lab)
        elif args.scenario == "connect_timeout": connect_timeout(lab)
        else: client_id_collision(lab)
        passed = True
    finally:
        lab.finish(passed)
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as error:
        print(f"INTEGRATION_FAIL: {error}", file=sys.stderr)
        sys.exit(1)
