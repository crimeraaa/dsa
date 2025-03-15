import math

PyLong_Shift = 30
PyLong_Base  = 1 << PyLong_Shift

# lookup table for all possible log_PyLong_Base(base)
# Assumes `2 <= base <= 36`
log_BASE_base: list[float] = [0.0] * 37

print(f"PyLong_Shift = {PyLong_Shift}")
print(f"PyLong_Base = {PyLong_Base}")

# https://github.com/python/cpython/blob/7ffe93faf1db3b90968af1b1d811f39529603780/Tools/scripts/long_conv_tables.py#L22
for base in range(0, 37):
    try:
        # log_b(a) / log_b(c)
        naive = math.log(base) / math.log(PyLong_Base)

        # log_b(a) / log_b(c) == log_c(a)
        simple = math.log(base, PyLong_Base)
        print(base, naive, 'and', simple)
        log_BASE_base[base] = naive
    except ValueError:
        log_BASE_base[base] = 0.0

def get_digit_count(n):
    """
    Given the implementation constant base `B` = 32,
    and variable input PyLong_Base `b` and input integer `n`,
    find the smallest integer `N` such that:

    Steps:
    1. B**N - 1 >= b**n - 1
        - next: add 1 on both sides

    2. (B**N - 1) + 1 >= (b**n - 1) + 1
        - next: simplify

    3. B**N >= b**n
        - next: isolate `N` by getting `log` (any PyLong_Base) of both sides

    4. log(B**N) >= log(b**n)
        - property of log (): log_B(N**x) == x * log_B(N)
        - thus, isolate the exponents

    5. N * log(B) >= n * log(b)
        - next: divide both sides by `log(B)`

    6. N >= n * log(b)/log(B)
        - property of log (change of quotient):
            - `log_b(a) / log_b(c) == log_c(a)`

    7. N >= n * log_B(b)
    """
    assert 0 < base <= 36, f"Invalid base '{base}'!"
    return n * log_BASE_base[base]

