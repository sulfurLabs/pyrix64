; SPDX-License-Identifier: GPL-3.0-or-later
; Copyright (C) 2026 sulfurLabs
;
; PROJECT: pyrix64 - examples
; FILE: bayle_player.asm
; DESC.: Bayle Video Showcase Player (160x100 Mode 13h scaled 4x)
;
; CREATED BY: offihito
; UPDATED BY: offihito
;

MOVI r10, 0x0E02    ; Pixel X address
MOVI r11, 0x0E03    ; Pixel Y address
MOVI r12, 0x0E04    ; Pixel Color address (plots & advances X)
MOVI r13, 0x0E05    ; Frame Sync address (flushes & sleeps)
MOVI r14, 0x0E06    ; Pixel Scale address
MOVI r15, 0x0E07    ; Video Stream address

; Set 4x pixel scaling (160x100 virtual resolution -> 640x400 window)
MOVI r1, 4
STORE r1, r14

frame_loop:
    ; Reset X=0, Y=0
    STORE zero, r10
    STORE zero, r11

    ; 160 x 100 = 16000 pixels per frame
    MOVI r2, 16000
    MOVI r3, 1

pixel_loop:
    LOAD r4, r15          ; Read 24-bit RGB pixel from video stream
    STORE r4, r12         ; Plot pixel (auto-advances X and wraps Y)
    SUB r2, r2, r3
    BNE r2, zero, pixel_loop

    ; Sync frame: sleep 40ms (~25 FPS)
    MOVI r1, 40
    STORE r1, r13

    JMP frame_loop

    HALT
