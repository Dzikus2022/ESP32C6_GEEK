#!/usr/bin/env python3
"""GEEK WiFi Sniffer v0.1 — USB binary → Radiotap/PCAP for Wireshark."""

from __future__ import annotations

import argparse
import os
import select
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
PCAP_MAGIC = 0xA1B2C3D4
PCAP_MAGIC_BYTES = bytes.fromhex("d4c3b2a1")
PCAP_VERSION_MAJOR = 2
PCAP_VERSION_MINOR = 4
PCAP_SNAPLEN = 65535
PCAP_GLOBAL_HEADER_LEN = 24
LINKTYPE_IEEE802_11_RADIOTAP = 127
IEEE80211_CHAN_2GHZ = 0x0080
PRESENT_FLAGS = 1 << 1
PRESENT_CHANNEL = 1 << 3
PRESENT_DBM_ANTSIGNAL = 1 << 5
IEEE80211_RADIOTAP_F_FCS = 0x10
RADIOTAP_FIXED_LEN = 8


def crc32_ieee(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF


def channel_to_mhz(channel: int) -> int:
    if 1 <= channel <= 14:
        return 2407 + 5 * channel
    return 0


def frame_has_trailing_fcs(pkt: dict) -> bool:
    """ESP32 sig_len includes FCS; only the full captured MPDU still has it."""
    frame = pkt.get("frame", b"")
    capt = len(frame)
    orig = int(pkt.get("orig_len", capt))
    return capt >= 4 and capt == orig


def build_radiotap(rssi: int, channel: int, fcs_at_end: bool = False) -> bytes:
    """Minimal Radiotap: Flags, Channel (if valid), dBm Antsignal.

    Field order follows present bits. Channel is u16-aligned from header start.
    FCS-at-end (IEEE80211_RADIOTAP_F_FCS) is set only when the 802.11 bytes
    still include the 4-byte FCS from ESP32 sig_len.
    """
    present = PRESENT_FLAGS | PRESENT_DBM_ANTSIGNAL
    flags = IEEE80211_RADIOTAP_F_FCS if fcs_at_end else 0
    body = struct.pack("<B", flags & 0xFF)
    freq = channel_to_mhz(channel)
    if freq != 0:
        present |= PRESENT_CHANNEL
        if (RADIOTAP_FIXED_LEN + len(body)) % 2:
            body += b"\x00"
        body += struct.pack("<HH", freq, IEEE80211_CHAN_2GHZ)
    body += struct.pack("<b", int(rssi))
    pad = (4 - ((RADIOTAP_FIXED_LEN + len(body)) % 4)) % 4
    body += b"\x00" * pad
    it_len = RADIOTAP_FIXED_LEN + len(body)
    header = struct.pack("<BBHI", 0, 0, it_len, present)
    return header + body


def parse_radiotap(data: bytes) -> dict:
    if len(data) < RADIOTAP_FIXED_LEN:
        raise ValueError("radiotap shorter than fixed header")
    version, _pad, it_len, present = struct.unpack_from("<BBHI", data, 0)
    if it_len < RADIOTAP_FIXED_LEN or it_len > len(data):
        raise ValueError("radiotap it_len out of range")
    offset = RADIOTAP_FIXED_LEN
    flags = None
    freq = None
    chan_flags = None
    rssi = None
    if present & PRESENT_FLAGS:
        flags = data[offset]
        offset += 1
    if present & PRESENT_CHANNEL:
        if offset % 2:
            offset += 1
        freq, chan_flags = struct.unpack_from("<HH", data, offset)
        offset += 4
    if present & PRESENT_DBM_ANTSIGNAL:
        rssi = struct.unpack_from("<b", data, offset)[0]
        offset += 1
    return {
        "version": version,
        "it_len": it_len,
        "present": present,
        "flags": flags,
        "freq": freq,
        "chan_flags": chan_flags,
        "rssi": rssi,
        "frame": data[it_len:],
    }


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
    return struct.pack(
        "<IHHIIII",
        PCAP_MAGIC,
        PCAP_VERSION_MAJOR,
        PCAP_VERSION_MINOR,
        0,
        0,
        PCAP_SNAPLEN,
        LINKTYPE_IEEE802_11_RADIOTAP,
    )


def write_pcap_global_header(stream) -> int:
    data = pcap_global_header()
    written = stream.write(data)
    stream.flush()
    if written not in (None, len(data)):
        raise RuntimeError("short PCAP global header write")
    return len(data)


def pcap_record(pkt: dict) -> bytes:
    frame = pkt["frame"]
    radiotap = build_radiotap(
        pkt["rssi"], pkt["channel"], frame_has_trailing_fcs(pkt)
    )
    incl = len(radiotap) + len(frame)
    orig = len(radiotap) + int(pkt["orig_len"])
    ts = int(pkt["timestamp_us"])
    sec = ts // 1_000_000
    usec = ts % 1_000_000
    return struct.pack("<IIII", sec, usec, incl, orig) + radiotap + frame


def write_pcap_record(stream, pkt: dict) -> int:
    data = pcap_record(pkt)
    written = stream.write(data)
    stream.flush()
    if written not in (None, len(data)):
        raise RuntimeError("short PCAP record write")
    return len(data)


def ensure_fifo(path: str) -> None:
    if os.path.exists(path):
        if not stat_is_fifo(path):
            raise SystemExit(f"{path} exists and is not a FIFO")
        return
    os.mkfifo(path, 0o644)


def open_fifo_session(path: str):
    """Block until a reader appears, then write a fresh PCAP global header."""
    sys.stderr.write(f"waiting for reader on {path}\n")
    stream = open(path, "wb", buffering=0)
    write_pcap_global_header(stream)
    sys.stderr.write(f"wrote PCAP global header to {path}\n")
    return stream


def fifo_reader_gone(stream) -> bool:
    try:
        poller = select.poll()
        poller.register(stream.fileno(), select.POLLERR | select.POLLHUP)
        events = poller.poll(0)
    except (ValueError, OSError):
        return True
    if not events:
        return False
    mask = events[0][1]
    return bool(mask & (select.POLLERR | select.POLLHUP))


def close_quietly(stream) -> None:
    if stream is None:
        return
    try:
        stream.close()
    except OSError:
        pass


def stat_is_fifo(path: str) -> bool:
    return stat_mode_fifo(os.stat(path).st_mode)


def stat_mode_fifo(mode: int) -> bool:
    return (mode & 0o170000) == 0o010000


def ieee80211_fcs(mpdu: bytes) -> bytes:
    return struct.pack("<I", zlib.crc32(mpdu) & 0xFFFFFFFF)


def sample_beacon_with_fcs() -> bytes:
    header = (
        b"\x80\x00\x00\x00"
        + (b"\xff" * 6)
        + b"\x02\x00\x00\x00\x00\x01"
        + b"\x02\x00\x00\x00\x00\x01"
        + b"\x00\x00"
    )
    fixed = (b"\x00" * 8) + b"\x64\x00\x01\x00"
    ssid = b"\x00\x04GEEK"
    rates = b"\x01\x08\x82\x84\x8b\x96\x0c\x12\x18\x24"
    mpdu = header + fixed + ssid + rates
    return mpdu + ieee80211_fcs(mpdu)


def sample_pcap_packet() -> dict:
    frame = sample_beacon_with_fcs()
    return {
        "seq": 1,
        "timestamp_us": 1_234_567,
        "channel": 6,
        "rssi": -40,
        "orig_len": len(frame),
        "capt_len": len(frame),
        "frame": frame,
    }


def validate_pcap_stream(data: bytes) -> None:
    if len(data) < PCAP_GLOBAL_HEADER_LEN:
        raise RuntimeError(f"PCAP too short: {len(data)} bytes")
    if data[:4] != PCAP_MAGIC_BYTES:
        raise RuntimeError(f"bad magic {data[:4].hex()}, expected d4c3b2a1")
    magic, major, minor, _tz, _sig, snap, network = struct.unpack(
        "<IHHIIII", data[:PCAP_GLOBAL_HEADER_LEN]
    )
    if magic != PCAP_MAGIC or major != 2 or minor != 4:
        raise RuntimeError(f"bad version {major}.{minor} magic=0x{magic:08x}")
    if snap != PCAP_SNAPLEN or network != LINKTYPE_IEEE802_11_RADIOTAP:
        raise RuntimeError(f"bad snap/network {snap}/{network}")
    textish = b"waiting" in data or b"reading" in data or b"ok seq=" in data
    if textish:
        raise RuntimeError("diagnostic text leaked into PCAP stream")
    if len(data) > PCAP_GLOBAL_HEADER_LEN:
        if len(data) < PCAP_GLOBAL_HEADER_LEN + 16:
            raise RuntimeError("truncated PCAP packet header")


def self_test() -> int:
    import subprocess
    import tempfile

    pkt = sample_pcap_packet()
    with tempfile.NamedTemporaryFile(prefix="geek-pcap-", suffix=".pcap", delete=False) as tmp:
        path = tmp.name
    try:
        with open(path, "wb") as stream:
            write_pcap_global_header(stream)
            write_pcap_record(stream, pkt)
        with open(path, "rb") as stream:
            data = stream.read()
        validate_pcap_stream(data)
        rec = pcap_record(pkt)
        if data[PCAP_GLOBAL_HEADER_LEN:] != rec:
            raise RuntimeError("packet record mismatch after global header")
        sys.stderr.write(
            f"self-test header=24 magic=d4c3b2a1 version=2.4 dlt=127 bytes={len(data)}\n"
        )
        capinfos = os.environ.get("CAPINFOS", "capinfos")
        tshark = os.environ.get("TSHARK", "tshark")
        if _have_cmd(capinfos):
            subprocess.run([capinfos, path], check=True)
        if _have_cmd(tshark):
            subprocess.run([tshark, "-r", path, "-c", "1"], check=True)
    finally:
        try:
            os.unlink(path)
        except OSError:
            pass
    return 0


def _have_cmd(name: str) -> bool:
    from shutil import which

    return which(name) is not None


def main() -> int:
    parser = argparse.ArgumentParser(description="GEEK WiFi sniffer USB → PCAP")
    parser.add_argument("--port", default="/dev/ttyACM0")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--fifo", default="/tmp/geek-wifi")
    parser.add_argument("--output", default=None, help="optional .pcap file")
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument(
        "--self-test",
        action="store_true",
        help="write a temp PCAP with the same helper and validate it",
    )
    args = parser.parse_args()

    if args.self_test:
        return self_test()

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

    ensure_fifo(args.fifo)
    ser = serial.Serial(args.port, args.baud, timeout=0.2)
    out = open(args.output, "wb") if args.output else None
    if out:
        write_pcap_global_header(out)

    fifo = None
    buf = bytearray()
    ok = 0
    bad = 0
    sys.stderr.write(f"reading {args.port} → {args.fifo}\n")
    try:
        while running:
            if fifo is None:
                fifo = open_fifo_session(args.fifo)
            elif fifo_reader_gone(fifo):
                sys.stderr.write("FIFO reader gone, will rewrite PCAP header\n")
                close_quietly(fifo)
                fifo = None
                continue

            chunk = ser.read(4096)
            if chunk:
                buf.extend(chunk)
            else:
                time.sleep(0.01)

            while fifo is not None:
                pkt, buf = try_extract(buf)
                if pkt is None:
                    break
                try:
                    write_pcap_record(fifo, pkt)
                except (BrokenPipeError, OSError):
                    sys.stderr.write("FIFO broken, will rewrite PCAP header\n")
                    close_quietly(fifo)
                    fifo = None
                    break
                if out:
                    write_pcap_record(out, pkt)
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
        close_quietly(fifo)
        if out:
            out.close()
        sys.stderr.write(f"done packets={ok} resyncs={bad}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
