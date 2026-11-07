; pyrix64 example: fibonacci sequence
; calculates F(n) for n = 7
; result in r2 = 13 (0xD)

MOVI r1, 7
MOVI r2, 0
MOVI r3, 1
MOVI r5, 1

fib_loop:
    BEQ r1, zero, fib_done
    ADD r4, r2, r3
    MOV r2, r3
    MOV r3, r4
    SUB r1, r1, r5
    JMP fib_loop

fib_done:
    HALT
