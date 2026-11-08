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

#include <inttypes.h>
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
        printf("::  error: register read out of range (r%u)\n", RegIndex);

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
        printf("::  error: register write out of range (r%u)\n", RegIndex);

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

//exception program counter
uint64_t Pyrix64EpcRead(
    const Pyrix64Cpu *Cpu
){
    return Cpu->Epc;
}
void Pyrix64EpcWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
){
    Cpu->Epc = Value;
}

//cause of interrupt/exception
uint64_t Pyrix64CauseRead(
    const Pyrix64Cpu *Cpu
){
    return Cpu->Cause;
}
void Pyrix64CauseWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
){
    Cpu->Cause = Value;
}

//trap vector
uint64_t Pyrix64TvecRead(
    const Pyrix64Cpu *Cpu
){
    return Cpu->Tvec;
}
void Pyrix64TvecWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
){
    Cpu->Tvec = Value;
}

//stack pointers
// one per privilege level, SP0-3

//reads a stack pointer
bool Pyrix64StackPointerRead(
    const Pyrix64Cpu *Cpu,
    uint8_t SpIndex,
    uint64_t *Value
){
    if (SpIndex >= Pyrix64StackPointerCount)
    {
        printf("::  error: stack pointer read out of range (sp%u)\n", SpIndex);

        return false;
    }

    *Value = Cpu->StackPointers[SpIndex];

    return true;
}

//writes a stack pointer
bool Pyrix64StackPointerWrite(
    Pyrix64Cpu *Cpu,
    uint8_t SpIndex,
    uint64_t Value
){
    if (SpIndex >= Pyrix64StackPointerCount)
    {
        printf("::  error: stack pointer write out of range (sp%u)\n", SpIndex);

        return false;
    }

    Cpu->StackPointers[SpIndex] = Value;

    return true;
}

//index of sp for current pl
// from CurrentPL
uint8_t Pyrix64ActiveStackPointerIndex(
    const Pyrix64Cpu *Cpu
){
    if ((uint8_t)Cpu->CurrentPL >= Pyrix64StackPointerCount)
    {
        //shouldnt ever happen
        // its validated when writing
        printf("::  error: no stack pointer for this privilege level\n");

        return 0;
    }

    return (uint8_t)Cpu->CurrentPL;
}

// reads sp of current pl
uint64_t Pyrix64ActiveStackPointerRead(
    const Pyrix64Cpu *Cpu
){
    return Cpu->StackPointers[Pyrix64ActiveStackPointerIndex(Cpu)];
}

//writes sp of current pl
void Pyrix64ActiveStackPointerWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
){
    Cpu->StackPointers[Pyrix64ActiveStackPointerIndex(Cpu)] = Value;
}

//read current pl
Pyrix64PrivilegeLevel Pyrix64PrivilegeLevelRead(
    const Pyrix64Cpu *Cpu
){
    return Cpu->CurrentPL;
}

//change current pl
//TODO:
// automatic stack switching
bool Pyrix64PrivilegeLevelWrite(
    Pyrix64Cpu *Cpu,
    Pyrix64PrivilegeLevel PL
){
    if ((uint8_t)PL >= Pyrix64PrivilegeLevelCount)
    {
        printf("::  error: invalid privilege level (%u)\n", (uint8_t)PL);

        return false;
    }

    Cpu->CurrentPL = PL;

    return true;
}

//instruction fetch
// first opcode tells length
bool Pyrix64FetchInstruction(
    Pyrix64Ram *Ram,
    uint64_t Pc,
    uint32_t *Word0,
    uint32_t *ExtWord,
    uint32_t *Length
){
    if (!Pyrix64RamLoad32(Ram, Pc, Word0))
    {
        return false;
    }

    *Length = Pyrix64InstructionLength(*Word0);

    if (*Length == Pyrix64InstrLen32)
    {
        //32bit no extention word
        *ExtWord = 0;

        return true;
    }

    // pc + 4 cant overflow
    if (!Pyrix64RamLoad32(Ram, Pc + Pyrix64InstrLen32, ExtWord))
    {
        printf("::  error: incomplete instruction; no second word (pc 0x%" PRIX64 ")\n", Pc);

        return false;
    }

    return true;
}

//instruction fetch
// instruction fetching
bool Pyrix64Execute(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram
){
    while (!Cpu->Halted)
    {
        // program counter
        if (Cpu->Pc >= Pyrix64RamSize)
        {
            printf("::  error: program counter out of range \n");

            return false;
        }

        uint32_t Word0 = 0;
        uint32_t ExtWord = 0;
        uint32_t Length = 0;
        uint64_t NextPc = 0;

        if (!Pyrix64FetchInstruction(Ram, Cpu->Pc, &Word0, &ExtWord, &Length))
        {
            printf("::  error: instruction fetch failed at pc 0x%" PRIX64 " \n", Cpu->Pc);

            return false;
        }

        if (!Pyrix64DecodeExecute(Cpu, Ram, Word0, ExtWord, &NextPc))
        {
            return false;
        }

        // pc moves only after the instruction ran ok
        Cpu->Pc = NextPc;
    }

    return true;
}
