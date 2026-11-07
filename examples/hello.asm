; pyrix64 - Hello World using .string and loop
;
; Screen MMIO Address: 0x0E00 (3584)

MOVI r1, message
MOVI r3, 0x0E00
MOVI r4, 1

print_loop:
    LOADB r2, r1
    BEQ r2, zero, print_done
    STORE r2, r3
    ADD r1, r1, r4
    JMP print_loop

print_done:
    HALT

message:
    .string "Hello, World from pyrix64!\n"
