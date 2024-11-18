from io import BytesIO
from itertools import count


def _xor(input: BytesIO, output: BytesIO, key: bytes) -> str:
    for i in count():
        data = input.read(1)
        if not data:
            break

        output.write(bytes([data[0] ^ key[i % 6]]))

    return "".join(f"{b:02X}" for b in key)


decrypt = _xor
encrypt = _xor
