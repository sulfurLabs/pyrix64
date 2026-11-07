/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: instr.c
 * DESC.: instructions impl
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "cpu.h"
#include "instr.h"
#include "pyrix/isa.h"

/* 31-24  |  23-19   |  18-14   |  13-0
 * opcode | register | reserved | address
 */

#define Pyrix64DecodeOpcode(Instruction)  ((uint8_t)(((Instruction) >> 24) & 0xFFu))
#define Pyrix64DecodeRd(Instruction)      ((uint8_t)(((Instruction) >> 19) & 0x1Fu))
#define Pyrix64DecodeRs1(Instruction)     ((uint8_t)(((Instruction) >> 14) & 0x1Fu))
#define Pyrix64DecodeRs2(Instruction)     ((uint8_t)(((Instruction) >> 9)  & 0x1Fu))
#define Pyrix64DecodeAddress(Instruction) ((uint16_t)((Instruction) & 0x3FFFu))

// false if error
typedef bool (*Pyrix64OpFn)(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
);

// 19 bit signed immediate as used by movI
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

static bool Pyrix64InstrNop(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Cpu;
    (void)Ram;
    (void)Instruction;

    return true;
}

static bool Pyrix64InstrHalt(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;
    (void)Instruction;

    Cpu->Halted = true;

    return true;
}

static bool Pyrix64InstrMov(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);

    Cpu->Registers[Rd] = Cpu->Registers[Rs1];

    return true;
}

static bool Pyrix64InstrAdd(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Cpu->Registers[Rd] = Cpu->Registers[Rs1] + Cpu->Registers[Rs2];

    return true;
}

static bool Pyrix64InstrSub(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Cpu->Registers[Rd] = Cpu->Registers[Rs1] - Cpu->Registers[Rs2];

    return true;
}

static bool Pyrix64InstrAnd(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Cpu->Registers[Rd] = Cpu->Registers[Rs1] & Cpu->Registers[Rs2];

    return true;
}

static bool Pyrix64InstrOr(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Cpu->Registers[Rd] = Cpu->Registers[Rs1] | Cpu->Registers[Rs2];

    return true;
}

static bool Pyrix64InstrXor(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Cpu->Registers[Rd] = Cpu->Registers[Rs1] ^ Cpu->Registers[Rs2];

    return true;
}

static bool Pyrix64InstrNot(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);

    Cpu->Registers[Rd] = ~Cpu->Registers[Rs1];

    return true;
}

static bool Pyrix64InstrMovI(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);

    Cpu->Registers[Rd] = (uint64_t)Pyrix64DecodeImmediate(Instruction);

    return true;
}

static bool Pyrix64InstrLoad(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint16_t Address = Pyrix64DecodeAddress(Instruction);
    uint64_t Value = 0;

    if (!Pyrix64RamLoad(Ram, Address, &Value))
    {
        return false;
    }

    Cpu->Registers[Rd] = Value;

    return true;
}

static bool Pyrix64InstrStore(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    uint8_t Source = Pyrix64DecodeRd(Instruction);
    uint16_t Address = Pyrix64DecodeAddress(Instruction);

    if (!Pyrix64RamStore(Ram, Address, Cpu->Registers[Source]))
    {
        return false;
    }

    return true;
}

//dispatch table
// opcode to instruction
static const Pyrix64OpFn Pyrix64DispatchTable[0x100] =
{
    [Pyrix64opNop]   = Pyrix64InstrNop,
    [Pyrix64opHalt]  = Pyrix64InstrHalt,
    [Pyrix64opMov]   = Pyrix64InstrMov,
    [Pyrix64opAdd]   = Pyrix64InstrAdd,
    [Pyrix64opSub]   = Pyrix64InstrSub,
    [Pyrix64opAnd]   = Pyrix64InstrAnd,
    [Pyrix64opOr]    = Pyrix64InstrOr,
    [Pyrix64opXor]   = Pyrix64InstrXor,
    [Pyrix64opNot]   = Pyrix64InstrNot,
    [Pyrix64opMovI]  = Pyrix64InstrMovI,
    [Pyrix64opLoad]  = Pyrix64InstrLoad,
    [Pyrix64opStore] = Pyrix64InstrStore

};

bool Pyrix64DecodeExecute(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    uint8_t Opcode = Pyrix64DecodeOpcode(Instruction);

    const Pyrix64OpFn Handler = Pyrix64DispatchTable[Opcode];

    if (Handler == NULL)
    {
        printf(":: error: unknown opcode 0x%02X\n", Opcode);

        return false;
    }

    return Handler(Cpu, Ram, Instruction);
}
