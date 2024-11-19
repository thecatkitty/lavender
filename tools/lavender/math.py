def complement(n: int, base: int = 10) -> int:
    sum = 0
    while n:
        sum += n % base
        n //= base

    return (base - (sum % base)) % base


def parity(n: int) -> int:
    result = 0
    while n:
        result ^= n & 1
        n >>= 1
    return result
