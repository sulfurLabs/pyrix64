; pyrix64 example: sum from 10 down to 1
; result in r2: 10 + 9 + ... + 1 = 55

MOVI r1, 10
MOVI r2, 0
MOVI r3, 1

loop:
    ADD r2, r2, r1
    SUB r1, r1, r3
    BNE r1, zero, loop

HALT
