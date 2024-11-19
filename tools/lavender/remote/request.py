def to_request_bytes(cid: int, uid: int, stamp: int) -> bytes:
    cid_bytes = cid.to_bytes(4, "little")
    uid_bytes = uid.to_bytes(10, "big")
    stamp_bytes = stamp.to_bytes(4, "little")

    code_bytes = list(b for b in stamp_bytes + cid_bytes + uid_bytes)
    for i in range(1, len(code_bytes)):
        code_bytes[i] ^= code_bytes[i - 1]

    return bytes(code_bytes)


def from_request_bytes(buffer: bytes) -> tuple[int, int, int]:
    buffer = list(buffer)
    for i in range(len(buffer) - 1, 0, -1):
        buffer[i] ^= buffer[i - 1]

    stamp = int.from_bytes(buffer[0:4], "little")
    cid = int.from_bytes(buffer[4:8], "little")
    uid = int.from_bytes(buffer[8:18], "big")
    return cid, uid, stamp
