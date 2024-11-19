from lavender.math import complement


def to_decimal_code(buffer: bytes) -> str:
    words = list(int.from_bytes(buffer[i:i+2], "little")
                 for i in range(0, len(buffer), 2))
    return "-".join(f"{complement(word)}{word:05d}" for word in words)


def from_decimal_code(code: str) -> bytes:
    groups = list(int(group) for group in code.split("-"))
    if any(complement(group) != 0 for group in groups):
        raise ValueError("Code has wrong parity")
    words = list(group % 100000 for group in groups)
    return list(b for w in words for b in w.to_bytes(2, "little"))
