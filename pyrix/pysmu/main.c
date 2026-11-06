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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cfg.h"
#include "pyrix/isa.h"

// general purpose registers
#define Pyrix64RegisterCount 32

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

typedef struct
{
    uint64_t Registers[Pyrix64RegisterCount]; // 64 bit
    uint32_t Pc; // pc == program counter, not personal computer
    bool Halted; // true if it really stopped

} Pyrix64Cpu;

static int64_t Pyrix64DecodeImmediate(uint32_t Instruction)
{
    int64_t Value = Instruction & 0x7FFFFu;

    // 19 bits
    if (Value & (1LL << 18))
    {
        Value -= (1LL << 19);
    }

    return Value;
}

static bool Pyrix64Execute(
    Pyrix64Cpu *Cpu,
    const uint32_t *Program,
    uint32_t ProgramSize
){
    while (!Cpu->Halted)
    {
        if (Cpu->Pc >= ProgramSize)
        {
            printf("program counter out of range \n");

            return false;
        }

        uint32_t Instruction = Program[Cpu->Pc++];
        uint8_t Opcode = (Instruction >> 24) & 0xFF;
        uint8_t Rd = (Instruction >> 19) & 0x1F;
        uint8_t Rs1 = (Instruction >> 14) & 0x1F;
        uint8_t Rs2 = (Instruction >> 9) & 0x1F;

        switch (Opcode)
        {
            case Pyrix64opNop:
                break;

            case Pyrix64opHalt:
                Cpu->Halted = true;

                break;

            case Pyrix64opMov:
                Cpu->Registers[Rd] = Cpu->Registers[Rs1];

                break;

            case Pyrix64opAdd:
                Cpu->Registers[Rd] = Cpu->Registers[Rs1] + Cpu->Registers[Rs2];

                break;

            case Pyrix64opSub:
                Cpu->Registers[Rd] = Cpu->Registers[Rs1] - Cpu->Registers[Rs2];

                break;

            case Pyrix64opAnd:
                Cpu->Registers[Rd] = Cpu->Registers[Rs1] & Cpu->Registers[Rs2];

                break;

            case Pyrix64opOr:
                Cpu->Registers[Rd] = Cpu->Registers[Rs1] | Cpu->Registers[Rs2];

                break;

            case Pyrix64opXor:
                Cpu->Registers[Rd] = Cpu->Registers[Rs1] ^ Cpu->Registers[Rs2];

                break;

            case Pyrix64opNot:
                Cpu->Registers[Rd] = ~Cpu->Registers[Rs1];

                break;

            case Pyrix64opMovI:
                Cpu->Registers[Rd] = (uint64_t)Pyrix64DecodeImmediate(Instruction);

                break;

            default:
                printf("Error: unknown opcode 0x%02X\n", Opcode);

                return false;
        }
    }

    return true;
}

int main(void)
{
    Pyrix64Cpu Cpu = {0};

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
        Pyrix64EncodeR(Pyrix64opHalt, 0, 0, 0)
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    printf("::  pyrix64 pysmu emulator\n");
    printf("::  starting program \n");

    if (!Pyrix64Execute(&Cpu, Program, ProgramSize))
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

    return 0;
}
