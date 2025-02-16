#!/usr/bin/python3

# For testing only
CAP = 16

while True:
    try:
        desired = int(input("Desired index: ")) % CAP
        actual  = int(input(" Actual index: ")) % CAP
        probe: int
        if actual >= desired:
            probe = actual - desired
        else:
            probe = (CAP % desired) + actual

        print(f"\tProbe of actual ({actual}) from desired ({desired}): {probe}")
    except ValueError as err:
        print(err)
        continue
    except EOFError:
        print()
        break


