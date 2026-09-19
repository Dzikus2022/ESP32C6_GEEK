#!/usr/bin/env python3
"""GEEK WiFi Sniffer v0.1 — USB binary → Radiotap/PCAP for Wireshark."""

from __future__ import annotations

import argparse
import os
import signal
import struct
import sys
import time
import zlib
from typing import Optional

MAGIC = b"GKW1"
VERSION = 1
HEADER_SIZE = 17
MAX_CAPTURE = 256
MAX_ORIG = 4095
LINKTYPE_IEEE802_11_RADIOTAP = 127
IEEE80211_CHAN_2GHZ = 0x0080
PRESENT_CHANNEL = 1 << 3
PRESENT_DBM_ANTSIGNAL = 1 << 5


def crc32_ieee(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF


def channel_to_mhz(channel: int) -> int:
    if 1 <= channel <= 14:
        return 2407 + 5 * channel
    return 0


def build_radiotap(rssi: int, channel: int) -> bytes:
    freq = channel_to_mhz(channel)
    present = PRESENT_DBM_ANTSIGNAL
    body = b""
    if freq != 0:
        present |= PRESENT_CHANNEL
        body += struct.pack("<HH", freq, IEEE80211_CHAN_2GHZ)
    body += struct.pack("<b", int(rssi))
    pad = (4 - ((8 + len(body)) % 4)) % 4
    body += b"\x00" * pad
    header = struct.pack("<BBHI", 0, 0, 8 + len(body), present)
    return header + body


def encode_capture(
    seq: int,
    timestamp_us: int,
    channel: int,
    rssi: int,
    orig_len: int,
    frame: bytes,
) -> bytes:
    capt = len(frame)
    header = bytearray()
    header += MAGIC
    header.append(VERSION)
    header += struct.pack("<H", seq & 0xFFFF)
    header += struct.pack("<I", timestamp_us & 0xFFFFFFFF)
    header.append(channel & 0xFF)
    header.append(rssi & 0xFF)
    header += struct.pack("<HH", orig_len & 0xFFFF, capt)
    payload = bytes(header[4:]) + frame
    return bytes(header) + frame + struct.pack("<I", crc32_ieee(payload))


def try_extract(buf: bytearray) -> tuple[Optional[dict], bytearray]:
    idx = buf.find(MAGIC)
    if idx < 0:
        keep = min(len(buf), len(MAGIC) - 1)
        del buf[: len(buf) - keep]
        return None, buf
    if idx > 0:
        del buf[:idx]
    if len(buf) < HEADER_SIZE:
        return None, buf

    version = buf[4]
    if version != VERSION:
        del buf[0]
        return None, buf

    orig_len = int.from_bytes(buf[13:15], "little")
    capt_len = int.from_bytes(buf[15:17], "little")
    if capt_len > MAX_CAPTURE or orig_len > MAX_ORIG or capt_len > orig_len:
        del buf[0]
        return None, buf

    total = HEADER_SIZE + capt_len + 4
    if len(buf) < total:
        return None, buf

    body = bytes(buf[4 : HEADER_SIZE + capt_len])
    crc = int.from_bytes(buf[HEADER_SIZE + capt_len : total], "little")
    if crc32_ieee(body) != crc:
        del buf[0]
        return None, buf

    pkt = {
        "seq": int.from_bytes(buf[5:7], "little"),
        "timestamp_us": int.from_bytes(buf[7:11], "little"),
        "channel": buf[11],
        "rssi": struct.unpack("<b", bytes([buf[12]]))[0],
        "orig_len": orig_len,
        "capt_len": capt_len,
        "frame": bytes(buf[HEADER_SIZE : HEADER_SIZE + capt_len]),
    }
    del buf[:total]
    return pkt, buf


def pcap_global_header() -> bytes:
    return struct.pack("<IHHIIII", 0xA1B2C3D4, 2, 4, 0, 0, 65535, LINKTYPE_IEEE802_11_RADIOTAP)


def pcap_record(pkt: dict) -> bytes:
    radiotap = build_radiotap(pkt["rssi"], pkt["channel"])
    frame = pkt["frame"]
    incl = len(radiotap) + len(frame)
    orig = len(radiotap) + int(pkt["orig_len"])
    ts = int(pkt["timestamp_us"])
    sec = ts // 1_000_000
    usec = ts % 1_000_000
    return struct.pack("<IIII", sec, usec, incl, orig) + radiotap + frame


def open_fifo(path: str):
    if os.path.exists(path):
        if not stat_is_fifo(path):
            raise SystemExit(f"{path} exists and is not a FIFO")
    else:
        os.mkfifo(path, 0o644)
    sys.stderr.write(f"waiting for reader on {path}\n")
    return open(path, "wb", buffering=0)


def stat_is_fifo(path: str) -> bool:
    return stat_mode_fifo(os.stat(path).st_mode)


def stat_mode_fifo(mode: int) -> bool:
    return (mode & 0o170000) == 0o010000


def main() -> int:
    parser = argparse.ArgumentParser(description="GEEK WiFi sniffer USB → PCAP")
    parser.add_argument("--port", default="/dev/ttyACM0")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--fifo", default="/tmp/geek-wifi")
    parser.add_argument("--output", default=None, help="optional .pcap file")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    try:
        import serial
    except ImportError:
        sys.stderr.write("pyserial missing: pip install pyserial\n")
        return 1

    running = True

    def handle_stop(_sig, _frame):
        nonlocal running
        running = False

    signal.signal(signal.SIGINT, handle_stop)
    signal.signal(signal.SIGTERM, handle_stop)

    ser = serial.Serial(args.port, args.baud, timeout=0.2)
    header = pcap_global_header()
    out = open(args.output, "wb") if args.output else None
    if out:
        out.write(header)
        out.flush()
    fifo = open_fifo(args.fifo)
    fifo.write(header)

    buf = bytearray()
    ok = 0
    bad = 0
    sys.stderr.write(f"reading {args.port} → {args.fifo}\n")
    try:
        while running:
            chunk = ser.read(4096)
            if chunk:
                buf.extend(chunk)
            else:
                time.sleep(0.01)
                continue
            while True:
                pkt, buf = try_extract(buf)
                if pkt is None:
                    break
                rec = pcap_record(pkt)
                try:
                    fifo.write(rec)
                except BrokenPipeError:
                    running = False
                    break
                if out:
                    out.write(rec)
                    out.flush()
                ok += 1
                if args.verbose:
                    sys.stderr.write(
                        f"ok seq={pkt['seq']} ch={pkt['channel']} "
                        f"rssi={pkt['rssi']} len={pkt['capt_len']}/{pkt['orig_len']}\n"
                    )
            if len(buf) > 65536:
                del buf[:-8]
                bad += 1
    finally:
        ser.close()
        fifo.close()
        if out:
            out.close()
        sys.stderr.write(f"done packets={ok} resyncs={bad}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
