#!/usr/bin/env python3
"""Host tests for GEEK capture framing / resync."""

import unittest

from geek_sniffer import encode_capture, try_extract


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


if __name__ == "__main__":
    unittest.main()
