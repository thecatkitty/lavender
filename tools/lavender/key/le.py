import re
from typing import Generator

KEY_LOW_PART = 0
KEY_LOW_PART_MASK = 0b111111111111111111111111
KEY_HIGH_PART = 24
KEY_HIGH_PART_MASK = 0b111111111111111111111111

LOCAL_HIGH_ROTATION = 30
LOCAL_HIGH_ROTATION_MASK = 0b11
LOCAL_HIGH_PART = 24
LOCAL_HIGH_PART_MASK = 0b111111
LOCAL_LOW_PART = 0
LOCAL_LOW_PART_MASK = 0b111111111111111111111111

EXTERNAL_LOW_ROTATION = 18
EXTERNAL_LOW_ROTATION_MASK = 0b11
EXTERNAL_HIGH_PART = 0
EXTERNAL_HIGH_PART_MASK = 0b111111111111111111
EXTERNAL_HIGH_PART_SHIFT = 6

ROTATIONS = [3, 7, 13, 19]


def rotl24(n: int, c: int) -> int:
    return (n << c | n >> (24 - c)) & 0xFFFFFF


def rotr24(n: int, c: int) -> int:
    return (n >> c | n << (24 - c)) & 0xFFFFFF


class LE32B6D(object):
    def __init__(self, key: bytes):
        if len(key) != 6:
            raise ValueError("Key is not 48-bit")

        self.key = bytes(key)

    def _encode(self, i: int) -> list[str]:
        key = int.from_bytes(self.key, "big")

        low_part = (key >> KEY_LOW_PART) & KEY_LOW_PART_MASK
        high_part = (key >> KEY_HIGH_PART) & KEY_HIGH_PART_MASK

        low_rotation_index = i % len(ROTATIONS)
        high_rotation_index = i // len(ROTATIONS)

        rot_low_part = rotl24(low_part, ROTATIONS[low_rotation_index])
        rot_high_part = rotl24(high_part, ROTATIONS[high_rotation_index])

        local = ((high_rotation_index & LOCAL_HIGH_ROTATION_MASK) << LOCAL_HIGH_ROTATION) | (
            (rot_high_part & LOCAL_HIGH_PART_MASK) << LOCAL_HIGH_PART) | ((rot_low_part & LOCAL_LOW_PART_MASK) << LOCAL_LOW_PART)
        external = ((low_rotation_index & EXTERNAL_LOW_ROTATION_MASK) << EXTERNAL_LOW_ROTATION) | (
            ((rot_high_part >> EXTERNAL_HIGH_PART_SHIFT) & EXTERNAL_HIGH_PART_MASK) << EXTERNAL_HIGH_PART)

        if external >= 1000000:
            return None

        return [f"{local >> 16:04X}-{local & 0xFFFF:04X}", f"{external:06d}"]

    def encode(self, parameter: str = None) -> Generator[list[str], None, None]:
        if parameter is not None:
            encoded = self._encode(int(parameter))
            if encoded is not None:
                yield encoded
            return

        for i in range(16):
            encoded = self._encode(i)
            if encoded is not None:
                yield encoded

    @staticmethod
    def decode(fkey: list[str]) -> "LE32B6D":
        local, external = fkey
        m = re.match("([0-9a-f]{4})-?([0-9a-f]{4})", local, re.I)
        if m is None:
            raise ValueError("Local part has a wrong format")

        local = int(m.group(1) + m.group(2), 16)
        external = int(external)

        low_part = (local >> LOCAL_LOW_PART) & LOCAL_LOW_PART_MASK
        high_part = (((external >> EXTERNAL_HIGH_PART) & EXTERNAL_HIGH_PART_MASK) <<
                     EXTERNAL_HIGH_PART_SHIFT) | ((local >> LOCAL_HIGH_PART) & LOCAL_HIGH_PART_MASK)

        low_rotation_index = (
            external >> EXTERNAL_LOW_ROTATION) & EXTERNAL_LOW_ROTATION_MASK
        high_rotation_index = (
            local >> LOCAL_HIGH_ROTATION) & LOCAL_HIGH_ROTATION_MASK

        low_part = rotr24(low_part, ROTATIONS[low_rotation_index])
        high_part = rotr24(high_part, ROTATIONS[high_rotation_index])

        key = ((high_part & KEY_HIGH_PART_MASK) << KEY_HIGH_PART) | (
            (low_part & KEY_LOW_PART_MASK) << KEY_LOW_PART)
        return LE32B6D(key.to_bytes(6, "big"))
