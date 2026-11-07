; SPDX-License-Identifier: GPL-3.0-or-later
; Copyright (C) 2026 sulfurLabs
;
; PROJECT: pyrix64 - examples
; FILE: vga_demo.asm
; DESC.: VGA Colors & Font Demo
;
; CREATED BY: offihito
; UPDATED BY: offihito
;
; Screen Char MMIO Address:  0x0E00 (3584)
; Screen Color MMIO Address: 0x0E01 (3585)

MOVI r1, title
MOVI r3, 0x0E00
MOVI r5, 0x0E01
MOVI r4, 1

; Set yellow text (color 14 = 0x0E) on black (0)
MOVI r6, 14
STORE r6, r5

print_title:
    LOADB r2, r1
    BEQ r2, zero, title_done
    STORE r2, r3
    ADD r1, r1, r4
    JMP print_title

title_done:
    ; Set bright cyan text (color 11 = 0x0B)
    MOVI r6, 11
    STORE r6, r5
    MOVI r1, text_cyan
    CALL print_str

    ; Set bright green text (color 10 = 0x0A)
    MOVI r6, 10
    STORE r6, r5
    MOVI r1, text_green
    CALL print_str

    ; Set bright red text (color 12 = 0x0C)
    MOVI r6, 12
    STORE r6, r5
    MOVI r1, text_red
    CALL print_str

    ; Set bright white text (color 15 = 0x0F) on blue background (1 = 0x10) -> 0x1F
    MOVI r6, 0x1F
    STORE r6, r5
    MOVI r1, text_white_on_blue
    CALL print_str

    ; Reset to standard light gray on black (0x07)
    MOVI r6, 7
    STORE r6, r5
    HALT

print_str:
    LOADB r2, r1
    BEQ r2, zero, print_str_done
    STORE r2, r3
    ADD r1, r1, r4
    JMP print_str

print_str_done:
    RET

title:
    .string "=== Pyrix64 VGA Text Mode Demo (80x25, 8x16 font) ===\n\n"

text_cyan:
    .string " [Color 11] Bright Cyan VGA text\n"

text_green:
    .string " [Color 10] Bright Green VGA text\n"

text_red:
    .string " [Color 12] Bright Red VGA text\n"

text_white_on_blue:
    .string " [Color 0x1F] Bright White on Blue Background \n"
