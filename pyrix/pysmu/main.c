/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: main.c
 * DESC.: pyrix64 emulator
 *
 * This file is part of pyrix64.
 *
 * pyrix64 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * pyrix64 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the LICENSE file for the full license text.
 * You can also find the GPL-3.0 license at:
 * https://www.gnu.org/licenses/gpl-3.0.html
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "cfg.h"
#include "cpu.h"
#include "pyrix/isa.h"

// opcode + destination register + register + register
#define Pyrix64EncodeR(Op, Rd, Rs1, Rs2)  \
    (((uint32_t)(Op) << 24) |             \
    ((uint32_t)(Rd)  << 19) |             \
    ((uint32_t)(Rs1) << 14) |             \
    ((uint32_t)(Rs2) << 9))

// opcode + destination register + immediate val
#define Pyrix64EncodeI(Op, Rd, Imm)       \
    (((uint32_t)(Op) << 24) |             \
    ((uint32_t)(Rd)  << 19) |             \
    ((uint32_t)(Imm) & 0x7FFFFu))

int main(void)
{
    Pyrix64Cpu Cpu = {0};
    Pyrix64Ram Ram;

    Pyrix64RamInit(&Ram);

    /* custom program till we have a compiler */
    const uint32_t Program[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 20),
        Pyrix64EncodeI(Pyrix64opMovI, 2, 6),
        Pyrix64EncodeR(Pyrix64opAdd, 3, 1, 2),
        Pyrix64EncodeR(Pyrix64opSub, 4, 1, 2),
        Pyrix64EncodeR(Pyrix64opAnd, 5, 1, 2),
        Pyrix64EncodeR(Pyrix64opOr, 6, 1, 2),
        Pyrix64EncodeR(Pyrix64opXor, 7, 1, 2),
        Pyrix64EncodeR(Pyrix64opNot, 8, 2, 0),
        Pyrix64EncodeI(Pyrix64opStore, 1, 0x100),
        Pyrix64EncodeI(Pyrix64opLoad, 9, 0x100),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    printf("::  pyrix64 pysmu emulator\n");
    printf("::  starting program \n");

    if (!Pyrix64Execute(&Cpu, &Ram, Program, ProgramSize))
    {
        return 1;
    }

    printf("==> Program finished\n\n");

    printf(" - R1 = %" PRIu64 "\n", Cpu.Registers[1]);
    printf(" - R2 = %" PRIu64 "\n", Cpu.Registers[2]);
    printf(" - R3 = %" PRIu64 "\n", Cpu.Registers[3]);
    printf(" - R4 = %" PRIu64 "\n", Cpu.Registers[4]);
    printf(" - R5 = %" PRIu64 "\n", Cpu.Registers[5]);
    printf(" - R6 = %" PRIu64 "\n", Cpu.Registers[6]);
    printf(" - R7 = %" PRIu64 "\n", Cpu.Registers[7]);

    printf(" - R8 = 0x%016" PRIX64 "\n", Cpu.Registers[8]);
    printf(" - R9 = %" PRIu64 "\n", Cpu.Registers[9]);

    return 0;
}
