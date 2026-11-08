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
#include "enc.h"
#include "pyrix/isa.h"

int main(void)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    /* custom program till we have a compiler */
    //random program....
    const uint32_t Program[] =
    {
        0x20080014, // MOVI r1, 20
        0x20100006, // MOVI r2, 6
        0x11184400, // ADD r3
        0x12204400, // SUB r4
        0x16408000, // NOT r8
        0x31080100, // STORE r1 (0x100)
        0x30480100, // LOAD r9 (0x100)
        0x20500001, // MOVI r10, 1
        0x20280003, // MOVI r5, 3
        0x20300000, // MOVI r6, 0
        0x11318200, // ADD r6
        0x12295400, // SUB r5
        0xE3014000, // BNE r5, r0
        0x01000000  // HALT
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    // program into ram
    for (uint32_t i = 0; i < ProgramSize; i++)
    {
        if (!Pyrix64RamStore32(&Ram, (uint64_t)i * Pyrix64InstrLen32, Program[i]))
        {
            printf("::  program does not fit into ram \n");

            return 1;
        }
    }

    printf("::  pyrix64 pysmu emulator\n");
    printf("::  starting program \n");

    if (!Pyrix64Execute(&Cpu, &Ram))
    {
        return 1;
    }

    printf("==> Program finished\n\n");

    printf("  > R1 = %" PRIu64 "\n",       Cpu.Registers[1]);
    printf("  > R2 = %" PRIu64 "\n",       Cpu.Registers[2]);
    printf("  > R3 = %" PRIu64 "\n",       Cpu.Registers[3]);
    printf("  > R4 = %" PRIu64 "\n",       Cpu.Registers[4]);
    printf("  > R5 = %" PRIu64 "\n",       Cpu.Registers[5]);
    printf("  > R6 = %" PRIu64 "\n",       Cpu.Registers[6]);
    printf("  > R7 = %" PRIu64 "\n",       Cpu.Registers[7]);
    printf("  > R8 = 0x%016" PRIX64 "\n",  Cpu.Registers[8]);
    printf("  > R9 = %" PRIu64 "\n",       Cpu.Registers[9]);

    return 0;
}
