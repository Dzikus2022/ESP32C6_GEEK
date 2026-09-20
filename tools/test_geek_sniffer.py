#!/usr/bin/env python3
"""Host tests for GEEK capture framing / resync."""

import io
import struct
import tempfile
import unittest
import zlib

from geek_sniffer import (
    IEEE80211_CHAN_2GHZ,
    IEEE80211_RADIOTAP_F_FCS,
    LINKTYPE_IEEE802_11_RADIOTAP,
    PCAP_GLOBAL_HEADER_LEN,
    PCAP_MAGIC,
    PCAP_MAGIC_BYTES,
    PRESENT_CHANNEL,
    PRESENT_DBM_ANTSIGNAL,
    PRESENT_FLAGS,
    build_radiotap,
    encode_capture,
    frame_has_trailing_fcs,
    parse_radiotap,
    pcap_record,
    sample_beacon_with_fcs,
    sample_pcap_packet,
    try_extract,
    validate_pcap_stream,
    write_pcap_global_header,
    write_pcap_record,
)


def frame(seq=1, ts=123456, ch=6, rssi=-40, orig=None, data=None):
    data = data if data is not None else bytes(range(24))
    orig = orig if orig is not None else len(data)
    return encode_capture(seq, ts, ch, rssi, orig, data)


class ParserTests(unittest.TestCase):
    def test_valid_packet(self):
        raw = frame()
        pkt, rest = try_extract(bytearray(raw))
        self.assertIsNotNone(pkt)
        self.assertEqual(pkt["seq"], 1)
        self.assertEqual(pkt["channel"], 6)
        self.assertEqual(pkt["rssi"], -40)
        self.assertEqual(pkt["capt_len"], 24)
        self.assertEqual(len(rest), 0)

    def test_garbage_before_magic(self):
        raw = b"xxxx" + frame(seq=2)
        pkt, rest = try_extract(bytearray(raw))
        self.assertIsNotNone(pkt)
        self.assertEqual(pkt["seq"], 2)
        self.assertEqual(len(rest), 0)

    def test_bad_crc(self):
        raw = bytearray(frame())
        raw[-1] ^= 0xFF
        original = len(raw)
        pkt, rest = try_extract(raw)
        self.assertIsNone(pkt)
        self.assertLess(len(rest), original)

    def test_invalid_length(self):
        raw = bytearray(frame())
        raw[15] = 0xFF
        raw[16] = 0xFF
        pkt, _rest = try_extract(raw)
        self.assertIsNone(pkt)

    def test_truncated_packet(self):
        raw = frame()
        pkt, rest = try_extract(bytearray(raw[:10]))
        self.assertIsNone(pkt)
        self.assertEqual(len(rest), 10)

    def test_two_consecutive(self):
        raw = frame(seq=3) + frame(seq=4, data=b"ABCDEFGH")
        buf = bytearray(raw)
        first, buf = try_extract(buf)
        second, buf = try_extract(buf)
        self.assertEqual(first["seq"], 3)
        self.assertEqual(second["seq"], 4)
        self.assertEqual(second["frame"], b"ABCDEFGH")
        self.assertEqual(len(buf), 0)

    def test_corrupt_then_valid(self):
        bad = bytearray(frame(seq=5))
        bad[10] ^= 0xAA
        raw = bytes(bad) + frame(seq=6)
        buf = bytearray(raw)
        found = None
        for _ in range(len(raw)):
            pkt, buf = try_extract(buf)
            if pkt is not None:
                found = pkt
                break
        self.assertIsNotNone(found)
        self.assertEqual(found["seq"], 6)

    def test_resync_after_partial_magic(self):
        raw = b"GK" + frame(seq=9)
        pkt, rest = try_extract(bytearray(raw))
        self.assertIsNotNone(pkt)
        self.assertEqual(pkt["seq"], 9)
        self.assertEqual(len(rest), 0)


class PcapWriterTests(unittest.TestCase):
    def test_global_header_on_memory_stream(self):
        stream = io.BytesIO()
        n = write_pcap_global_header(stream)
        data = stream.getvalue()
        self.assertEqual(n, 24)
        self.assertEqual(len(data), PCAP_GLOBAL_HEADER_LEN)
        self.assertEqual(data[:4], PCAP_MAGIC_BYTES)
        self.assertEqual(data[:4], bytes.fromhex("d4c3b2a1"))
        magic, major, minor, tz, sig, snap, network = struct.unpack("<IHHIIII", data)
        self.assertEqual(magic, PCAP_MAGIC)
        self.assertEqual((major, minor), (2, 4))
        self.assertEqual((tz, sig), (0, 0))
        self.assertEqual(snap, 65535)
        self.assertEqual(network, LINKTYPE_IEEE802_11_RADIOTAP)
        validate_pcap_stream(data)

    def test_header_then_packet_no_text(self):
        stream = io.BytesIO()
        write_pcap_global_header(stream)
        pkt = sample_pcap_packet()
        write_pcap_record(stream, pkt)
        data = stream.getvalue()
        validate_pcap_stream(data)
        rec = pcap_record(pkt)
        self.assertEqual(data[PCAP_GLOBAL_HEADER_LEN : PCAP_GLOBAL_HEADER_LEN + 16], rec[:16])
        self.assertEqual(data[PCAP_GLOBAL_HEADER_LEN:], rec)
        self.assertNotIn(b"waiting", data)
        self.assertNotIn(b"reading", data)
        self.assertNotIn(b"ok seq=", data)
        ts_sec = struct.unpack_from("<I", rec, 0)[0]
        self.assertNotEqual(data[:4], struct.pack("<I", ts_sec))

    def test_temp_file_starts_with_classic_magic(self):
        pkt = sample_pcap_packet()
        with tempfile.NamedTemporaryFile(prefix="geek-pcap-", suffix=".pcap") as tmp:
            write_pcap_global_header(tmp)
            write_pcap_record(tmp, pkt)
            tmp.flush()
            tmp.seek(0)
            data = tmp.read()
        self.assertEqual(data[:4], b"\xd4\xc3\xb2\xa1")
        validate_pcap_stream(data)


class RadiotapFcsTests(unittest.TestCase):
    def test_full_frame_has_trailing_fcs(self):
        frame = sample_beacon_with_fcs()
        pkt = {
            "orig_len": len(frame),
            "capt_len": len(frame),
            "frame": frame,
        }
        self.assertTrue(frame_has_trailing_fcs(pkt))
        self.assertEqual(
            frame[-4:], struct.pack("<I", zlib.crc32(frame[:-4]) & 0xFFFFFFFF)
        )

    def test_truncated_frame_does_not_claim_fcs(self):
        frame = sample_beacon_with_fcs()
        pkt = {
            "orig_len": len(frame),
            "capt_len": len(frame) - 8,
            "frame": frame[:-8],
        }
        self.assertFalse(frame_has_trailing_fcs(pkt))
        rt = build_radiotap(-40, 6, False)
        parsed = parse_radiotap(rt)
        self.assertEqual(parsed["flags"], 0)

    def test_radiotap_flags_fcs_and_lengths(self):
        pkt = sample_pcap_packet()
        rec = pcap_record(pkt)
        ts_sec, ts_usec, incl, orig = struct.unpack_from("<IIII", rec, 0)
        payload = rec[16:]
        parsed = parse_radiotap(payload)
        self.assertEqual(parsed["it_len"], 16)
        self.assertEqual(
            parsed["present"],
            PRESENT_FLAGS | PRESENT_CHANNEL | PRESENT_DBM_ANTSIGNAL,
        )
        self.assertEqual(parsed["flags"], IEEE80211_RADIOTAP_F_FCS)
        self.assertEqual(parsed["freq"], 2437)
        self.assertEqual(parsed["chan_flags"], IEEE80211_CHAN_2GHZ)
        self.assertEqual(parsed["rssi"], -40)
        self.assertEqual(parsed["frame"], pkt["frame"])
        self.assertEqual(incl, parsed["it_len"] + len(pkt["frame"]))
        self.assertEqual(orig, parsed["it_len"] + pkt["orig_len"])
        self.assertEqual(len(rec), 16 + incl)
        self.assertEqual(ts_sec, 1)
        self.assertEqual(ts_usec, 234567)

    def test_control_frame_with_fcs_keeps_payload(self):
        ack = bytes([0xD4, 0x00, 0x00, 0x00]) + (b"\x02" * 6)
        frame = ack + b"\x11\x22\x33\x44"
        pkt = {
            "timestamp_us": 0,
            "channel": 6,
            "rssi": -70,
            "orig_len": len(frame),
            "capt_len": len(frame),
            "frame": frame,
        }
        rec = pcap_record(pkt)
        parsed = parse_radiotap(rec[16:])
        self.assertEqual(parsed["flags"], IEEE80211_RADIOTAP_F_FCS)
        self.assertEqual(parsed["frame"], frame)
        self.assertEqual(parsed["rssi"], -70)

    def test_tshark_decodes_ssid_without_malformed(self):
        import shutil
        import subprocess

        tshark = shutil.which("tshark")
        if tshark is None:
            self.skipTest("tshark not installed")
        pkt = sample_pcap_packet()
        with tempfile.NamedTemporaryFile(prefix="geek-beacon-", suffix=".pcap") as tmp:
            write_pcap_global_header(tmp)
            write_pcap_record(tmp, pkt)
            tmp.flush()
            out = subprocess.check_output(
                [
                    tshark,
                    "-r",
                    tmp.name,
                    "-T",
                    "fields",
                    "-e",
                    "wlan.ssid",
                    "-e",
                    "radiotap.dbm_antsignal",
                    "-e",
                    "radiotap.channel.freq",
                    "-e",
                    "radiotap.flags.fcs",
                    "-e",
                    "_ws.malformed",
                ],
                text=True,
            )
        fields = out.strip().split("\t")
        self.assertGreaterEqual(len(fields), 4)
        self.assertEqual(fields[0], "GEEK")
        self.assertEqual(fields[1], "-40")
        self.assertEqual(fields[2], "2437")
        self.assertIn(fields[3], ("1", "True", "true"))
        malformed = fields[4] if len(fields) > 4 else ""
        self.assertEqual(malformed, "")


if __name__ == "__main__":
    unittest.main()
