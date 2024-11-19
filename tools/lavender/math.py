def parity(n: int) -> int:
    result = 0
    while n:
        result ^= n & 1
        n >>= 1
    return result
