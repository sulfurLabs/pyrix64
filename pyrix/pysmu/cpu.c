/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: cpu.c
 * DESC.: central processing unit (cpu)
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

#include <stdio.h>
#include <string.h>

#include "cpu.h"
#include "instr.h"

//resets cpu state
void Pyrix64CpuInit(Pyrix64Cpu *Cpu)
{
    memset(Cpu, 0, sizeof(Pyrix64Cpu));
}

//reads a register
// r0 is always zero, like riscv
uint64_t Pyrix64RegRead(
    const Pyrix64Cpu *Cpu,
    uint8_t RegIndex
){
    if (RegIndex >= Pyrix64RegisterCount)
    {
        printf(":: error: register read out of range (r%u)\n", RegIndex);

        return 0;
    }
    if (RegIndex == Pyrix64RegisterZero)
    {
        return 0;
    }

    return Cpu->Registers[RegIndex];
}

//writes a register
// cuz r0 is zero, writes to r0 r ignored
void Pyrix64RegWrite(
    Pyrix64Cpu *Cpu,
    uint8_t RegIndex,
    uint64_t Value
) {
    if (RegIndex >= Pyrix64RegisterCount)
    {
        printf(":: error: register write out of range (r%u)\n", RegIndex);

        return;
    }
    if (RegIndex == Pyrix64RegisterZero)
    {
        return;
    }

    Cpu->Registers[RegIndex] = Value;
}

//status register

uint64_t Pyrix64StatusRead(
    const Pyrix64Cpu *Cpu
){
    return Cpu->Status;
}
void Pyrix64StatusWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
){
    Cpu->Status = Value;
}

//instruction fetch
bool Pyrix64Execute(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    const uint32_t *Program,
    uint32_t ProgramSize
){
    while (!Cpu->Halted)
    {
        // program counter
        if (Cpu->Pc >= ProgramSize)
        {
            printf("program counter out of range \n");

            return false;
        }

        uint32_t Instruction = Program[Cpu->Pc++];

        if (!Pyrix64DecodeExecute(Cpu, Ram, Instruction))
        {
            return false;
        }
    }

    return true;
}
