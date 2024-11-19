def _hash(request: bytes) -> int:
    a = int.from_bytes(request[0:14], "big")
    b = int.from_bytes(request[2:16], "big")
    c = int.from_bytes(request[4:18], "big")
    return a ^ b ^ c


def to_response(request: bytes, key: int) -> bytes:
    response = key ^ _hash(request)
    return response.to_bytes(14, "big")


def from_response(request: bytes, response: bytes) -> int:
    return int.from_bytes(response, "big") ^ _hash(request)
