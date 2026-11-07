/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: instr.c
 * DESC.: instructions impl
 *
 * CREATED BY: emex
 * UPDATED BY: offihito
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
#include "display.h"
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

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1));

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

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) + Pyrix64RegRead(Cpu, Rs2));

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

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) - Pyrix64RegRead(Cpu, Rs2));

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

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) & Pyrix64RegRead(Cpu, Rs2));

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

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) | Pyrix64RegRead(Cpu, Rs2));

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

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) ^ Pyrix64RegRead(Cpu, Rs2));

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

    Pyrix64RegWrite(Cpu, Rd, ~Pyrix64RegRead(Cpu, Rs1));

    return true;
}

static bool Pyrix64InstrMovI(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);

    Pyrix64RegWrite(Cpu, Rd, (uint64_t)Pyrix64DecodeImmediate(Instruction));

    return true;
}

static bool Pyrix64InstrLoad(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint64_t Address = Pyrix64RegRead(Cpu, Rs1) + Pyrix64DecodeAddress(Instruction);
    uint64_t Value = 0;

    if (Address == Pyrix64VideoStreamAddress)
    {
        Value = (uint64_t)Pyrix64VideoReadPixelRGB();
        Pyrix64RegWrite(Cpu, Rd, Value);

        return true;
    }

    if (!Pyrix64RamLoad(Ram, Address, &Value))
    {
        return false;
    }

    Pyrix64RegWrite(Cpu, Rd, Value);

    return true;
}

static bool Pyrix64InstrStore(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    uint8_t Source = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint64_t Address = Pyrix64RegRead(Cpu, Rs1) + Pyrix64DecodeAddress(Instruction);
    uint64_t Value = Pyrix64RegRead(Cpu, Source);

    if (Address == Pyrix64ScreenAddress)
    {
        Pyrix64DisplayPutChar((char)Value);
        Pyrix64DisplayFlush();

        return true;
    }
    else if (Address == Pyrix64ScreenColorAddress)
    {
        Pyrix64DisplaySetColor((uint8_t)(Value & 0x0F), (uint8_t)((Value >> 4) & 0x0F));

        return true;
    }
    else if (Address == Pyrix64PixelXAddress)
    {
        Pyrix64DisplaySetPixelX((uint32_t)Value);

        return true;
    }
    else if (Address == Pyrix64PixelYAddress)
    {
        Pyrix64DisplaySetPixelY((uint32_t)Value);

        return true;
    }
    else if (Address == Pyrix64PixelColorAddress)
    {
        Pyrix64DisplayPlotColorRGB((uint32_t)Value);

        return true;
    }
    else if (Address == Pyrix64FrameSyncAddress)
    {
        Pyrix64DisplayFrameSync((uint32_t)Value);

        if (!Pyrix64DisplayPoll())
        {
            Cpu->Halted = true;
        }

        return true;
    }
    else if (Address == Pyrix64PixelScaleAddress)
    {
        Pyrix64DisplaySetPixelScale((uint32_t)Value);

        return true;
    }
    else if (Address == Pyrix64VideoStreamAddress)
    {
        Pyrix64VideoRewind();

        return true;
    }

    if (!Pyrix64RamStore(Ram, Address, Value))
    {
        return false;
    }

    return true;
}

static bool Pyrix64InstrLoadB(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint64_t Address = Pyrix64RegRead(Cpu, Rs1) + Pyrix64DecodeAddress(Instruction);
    uint8_t ByteVal = 0;

    if (Address == Pyrix64VideoStreamAddress)
    {
        ByteVal = Pyrix64VideoReadPixelVGA();
        Pyrix64RegWrite(Cpu, Rd, (uint64_t)ByteVal);

        return true;
    }

    if (!Pyrix64RamLoadByte(Ram, Address, &ByteVal))
    {
        return false;
    }

    Pyrix64RegWrite(Cpu, Rd, (uint64_t)ByteVal);

    return true;
}

static bool Pyrix64InstrStoreB(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    uint8_t Source = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint64_t Address = Pyrix64RegRead(Cpu, Rs1) + Pyrix64DecodeAddress(Instruction);
    uint8_t Value = (uint8_t)Pyrix64RegRead(Cpu, Source);

    if (Address == Pyrix64ScreenAddress)
    {
        Pyrix64DisplayPutChar((char)Value);
        Pyrix64DisplayFlush();

        return true;
    }
    else if (Address == Pyrix64ScreenColorAddress)
    {
        Pyrix64DisplaySetColor((uint8_t)(Value & 0x0F), (uint8_t)((Value >> 4) & 0x0F));

        return true;
    }
    else if (Address == Pyrix64PixelXAddress)
    {
        Pyrix64DisplaySetPixelX((uint32_t)Value);

        return true;
    }
    else if (Address == Pyrix64PixelYAddress)
    {
        Pyrix64DisplaySetPixelY((uint32_t)Value);

        return true;
    }
    else if (Address == Pyrix64PixelColorAddress)
    {
        Pyrix64DisplayPlotColorVGA(Value);

        return true;
    }
    else if (Address == Pyrix64FrameSyncAddress)
    {
        Pyrix64DisplayFrameSync((uint32_t)Value);

        if (!Pyrix64DisplayPoll())
        {
            Cpu->Halted = true;
        }

        return true;
    }
    else if (Address == Pyrix64PixelScaleAddress)
    {
        Pyrix64DisplaySetPixelScale((uint32_t)Value);

        return true;
    }
    else if (Address == Pyrix64VideoStreamAddress)
    {
        Pyrix64VideoRewind();

        return true;
    }

    if (!Pyrix64RamStoreByte(Ram, Address, Value))
    {
        return false;
    }

    return true;
}

static bool Pyrix64InstrJmp(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    Cpu->Pc = Pyrix64DecodeAddress(Instruction);

    return true;
}

static bool Pyrix64InstrBeq(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rs1 = Pyrix64DecodeRd(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs1(Instruction);

    if (Pyrix64RegRead(Cpu, Rs1) == Pyrix64RegRead(Cpu, Rs2))
    {
        Cpu->Pc = Pyrix64DecodeAddress(Instruction);
    }

    return true;
}

static bool Pyrix64InstrBne(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    uint8_t Rs1 = Pyrix64DecodeRd(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs1(Instruction);

    if (Pyrix64RegRead(Cpu, Rs1) != Pyrix64RegRead(Cpu, Rs2))
    {
        Cpu->Pc = Pyrix64DecodeAddress(Instruction);
    }

    return true;
}

static bool Pyrix64InstrCall(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;

    Pyrix64RegWrite(Cpu, Pyrix64RegisterRa, Cpu->Pc);
    Cpu->Pc = Pyrix64DecodeAddress(Instruction);

    return true;
}

static bool Pyrix64InstrRet(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    (void)Ram;
    (void)Instruction;

    Cpu->Pc = Pyrix64RegRead(Cpu, Pyrix64RegisterRa);

    return true;
}

static bool Pyrix64InstrPush(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    if (Cpu->Sp < Pyrix64RamAccessSize)
    {
        printf(":: error: stack overflow\n");

        return false;
    }

    uint8_t Source = Pyrix64DecodeRd(Instruction);

    Cpu->Sp -= Pyrix64RamAccessSize;

    if (!Pyrix64RamStore(Ram, Cpu->Sp, Pyrix64RegRead(Cpu, Source)))
    {
        return false;
    }

    return true;
}

static bool Pyrix64InstrPop(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction
){
    if (Cpu->Sp + Pyrix64RamAccessSize > Pyrix64RamSize)
    {
        printf(":: error: stack underflow\n");

        return false;
    }

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint64_t Value = 0;

    if (!Pyrix64RamLoad(Ram, Cpu->Sp, &Value))
    {
        return false;
    }

    Cpu->Sp += Pyrix64RamAccessSize;
    Pyrix64RegWrite(Cpu, Rd, Value);

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
    [Pyrix64opLoad]   = Pyrix64InstrLoad,
    [Pyrix64opStore]  = Pyrix64InstrStore,
    [Pyrix64opLoadB]  = Pyrix64InstrLoadB,
    [Pyrix64opStoreB] = Pyrix64InstrStoreB,
    [Pyrix64opJmp]   = Pyrix64InstrJmp,
    [Pyrix64opBeq]   = Pyrix64InstrBeq,
    [Pyrix64opBne]   = Pyrix64InstrBne,
    [Pyrix64opCall]  = Pyrix64InstrCall,
    [Pyrix64opRet]   = Pyrix64InstrRet,
    [Pyrix64opPush]  = Pyrix64InstrPush,
    [Pyrix64opPop]   = Pyrix64InstrPop

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
