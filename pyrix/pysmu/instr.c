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

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "cpu.h"
#include "instr.h"
#include "pyrix/isa.h"

/* 31-24  |  23-19   |  18-14   |  13-0
 * opcode | register | reserved | address
 *
 * opcode chooses instruction lenth
 * 32 bit, one word
 * 64 bit, sec word (in ExtWord)
 * second word meaning depends on the instruction
 */

#define Pyrix64DecodeOpcode(Instruction)  ((uint8_t)(((Instruction) >> 24) & 0xFFu))
#define Pyrix64DecodeRd(Instruction)      ((uint8_t)(((Instruction) >> 19) & 0x1Fu))
#define Pyrix64DecodeRs1(Instruction)     ((uint8_t)(((Instruction) >> 14) & 0x1Fu))
#define Pyrix64DecodeRs2(Instruction)     ((uint8_t)(((Instruction) >> 9)  & 0x1Fu))
#define Pyrix64DecodeAddress(Instruction) ((uint16_t)((Instruction) & 0x3FFFu))

//length in bytes, chosen by the opcode and
// classic isa is 32 bit but the op64 and up are 64 bit
uint32_t Pyrix64InstructionLength(uint32_t Instruction)
{
    const uint8_t Opcode = Pyrix64DecodeOpcode(Instruction);

    if (Opcode >= Pyrix64Op64Base)
    {
        return Pyrix64InstrLen64;
    }

    return Pyrix64InstrLen32;
}

// false if error
typedef bool (*Pyrix64OpFn)(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
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
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Cpu;
    (void)Ram;
    (void)Instruction;
    (void)ExtWord;

    return true;
}

static bool Pyrix64InstrHalt(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Ram;
    (void)Instruction;
    (void)ExtWord;

    Cpu->Halted = true;

    return true;
}

static bool Pyrix64InstrMov(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Ram;
    (void)ExtWord;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1));

    return true;
}

static bool Pyrix64InstrAdd(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Ram;
    (void)ExtWord;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) + Pyrix64RegRead(Cpu, Rs2));

    return true;
}

static bool Pyrix64InstrSub(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Ram;
    (void)ExtWord;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) - Pyrix64RegRead(Cpu, Rs2));

    return true;
}

static bool Pyrix64InstrAnd(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Ram;
    (void)ExtWord;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) & Pyrix64RegRead(Cpu, Rs2));

    return true;
}

static bool Pyrix64InstrOr(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Ram;
    (void)ExtWord;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) | Pyrix64RegRead(Cpu, Rs2));

    return true;
}

static bool Pyrix64InstrXor(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Ram;
    (void)ExtWord;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    Pyrix64RegWrite(Cpu, Rd, Pyrix64RegRead(Cpu, Rs1) ^ Pyrix64RegRead(Cpu, Rs2));

    return true;
}

static bool Pyrix64InstrNot(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Ram;
    (void)ExtWord;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);

    Pyrix64RegWrite(Cpu, Rd, ~Pyrix64RegRead(Cpu, Rs1));

    return true;
}

static bool Pyrix64InstrMovI(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Ram;
    (void)ExtWord;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);

    Pyrix64RegWrite(Cpu, Rd, (uint64_t)Pyrix64DecodeImmediate(Instruction));

    return true;
}

static bool Pyrix64InstrLoad(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)ExtWord;

    uint8_t Rd = Pyrix64DecodeRd(Instruction);
    uint16_t Address = Pyrix64DecodeAddress(Instruction);
    uint64_t Value = 0;

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
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)ExtWord;

    uint8_t Source = Pyrix64DecodeRd(Instruction);
    uint16_t Address = Pyrix64DecodeAddress(Instruction);

    if (!Pyrix64RamStore(Ram, Address, Pyrix64RegRead(Cpu, Source)))
    {
        return false;
    }

    return true;
}

//push value to active stack
// stack goes down
// one entry is 8bytes, aligned!
// sp update
static bool Pyrix64StackPush(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint64_t Value
){
    const uint64_t StackAddress = Pyrix64ActiveStackPointerRead(Cpu) - Pyrix64StackEntrySize;

    if (StackAddress & (Pyrix64StackAlignment - 1))
    {
        printf("::  error: PUSH unaligned stack address (0x%016" PRIX64 ")\n", StackAddress);

        return false;
    }

    if (!Pyrix64RamStore(Ram, StackAddress, Value))
    {
        return false;
    }

    Pyrix64ActiveStackPointerWrite(Cpu, StackAddress);

    return true;
}

//pop value from active stack
// reads at current sp
// after sucessful load, sp gets update
static bool Pyrix64StackPop(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint64_t *Value
){
    const uint64_t StackAddress = Pyrix64ActiveStackPointerRead(Cpu);

    if (StackAddress & (Pyrix64StackAlignment - 1))
    {
        printf("::  error: POP unaligned stack address (0x%016" PRIX64 ")\n", StackAddress);

        return false;
    }

    if (!Pyrix64RamLoad(Ram, StackAddress, Value))
    {
        return false;
    }

    Pyrix64ActiveStackPointerWrite(Cpu, StackAddress + Pyrix64StackEntrySize);

    return true;
}

//push r to active stack
static bool Pyrix64InstrPush(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)ExtWord;

    const uint8_t Source = Pyrix64DecodeRd(Instruction);

    // push r0 is always 0
    return Pyrix64StackPush(Cpu, Ram, Pyrix64RegRead(Cpu, Source));
}

//pop r from active stack
static bool Pyrix64InstrPop(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)ExtWord;

    const uint8_t Dest = Pyrix64DecodeRd(Instruction);
    uint64_t Value = 0;

    if (!Pyrix64StackPop(Cpu, Ram, &Value))
    {
        return false;
    }

    Pyrix64RegWrite(Cpu, Dest, Value);

    return true;
}

//push fp on active stack
static bool Pyrix64InstrPushFp(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Instruction;
    (void)ExtWord;

    //fp doesnt count as gpr, reads and writes directly
    return Pyrix64StackPush(Cpu, Ram, Cpu->Fp);
}

//pop fp from active stack
static bool Pyrix64InstrPopFp(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Instruction;
    (void)ExtWord;

    uint64_t Value = 0;

    if (!Pyrix64StackPop(Cpu, Ram, &Value))
    {
        return false;
    }

    Cpu->Fp = Value;

    return true;
}

//the number of selected r in pushm/popm
// r0 is bit 0 and is ignored like always
static uint32_t Pyrix64DecodeMaskCount(uint32_t Mask)
{
    uint32_t Count = 0;

    for (uint32_t r = 1; r < Pyrix64RegisterCount; r++)
    {
        if (Mask & (1u << r))
        {
            Count++;
        }
    }

    return Count;
}

static bool Pyrix64InstrPushM(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Instruction;

    const uint32_t Mask = ExtWord & ~1u;

    if (Mask == 0)
    {
        return true;
    }

    const uint64_t SpStart = Pyrix64ActiveStackPointerRead(Cpu);
    const uint64_t Size = (uint64_t)Pyrix64DecodeMaskCount(Mask) * Pyrix64StackEntrySize;

    if (SpStart & (Pyrix64StackAlignment - 1))
    {
        printf("::  error: PUSHM unaligned stack address (0x%016" PRIX64 ")\n", SpStart);

        return false;
    }
    if (!Pyrix64RamRangeOk(SpStart - Size, Size))
    {
        printf("::  error: PUSHM stack range out of bounds (0x%016" PRIX64 ")\n", SpStart - Size);

        return false;
    }

    // r1, r2, ...
    uint64_t Address = SpStart;

    for (uint32_t r = 1; r < Pyrix64RegisterCount; r++)
    {
        if (Mask & (1u << r))
        {
            Address -= Pyrix64StackEntrySize;

            // cant fail
            if (!Pyrix64RamStore(Ram, Address, Pyrix64RegRead(Cpu, r)))
            {
                return false;
            }
        }
    }

    // in the end, sp is written
    Pyrix64ActiveStackPointerWrite(Cpu, Address);

    return true;
}
static bool Pyrix64InstrPopM(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)NextPc;
    (void)Instruction;

    const uint32_t Mask = ExtWord & ~1u;

    if (Mask == 0)
    {
        return true;
    }

    const uint64_t SpStart = Pyrix64ActiveStackPointerRead(Cpu);
    const uint64_t Size = (uint64_t)Pyrix64DecodeMaskCount(Mask) * Pyrix64StackEntrySize;

    if (SpStart & (Pyrix64StackAlignment - 1))
    {
        printf("::  error: POPM unaligned stack address (0x%016" PRIX64 ")\n", SpStart);

        return false;
    }
    if (!Pyrix64RamRangeOk(SpStart, Size))
    {
        printf("::  error: POPM stack range out of bounds (0x%016" PRIX64 ")\n", SpStart);

        return false;
    }

    uint64_t Values[Pyrix64RegisterCount] = { 0 };
    uint64_t Address = SpStart;

    for (uint32_t r = Pyrix64RegisterCount - 1; r >= 1; r--)
    {
        if (Mask & (1u << r))
        {
            //this cant fail like pushm
            // the range is checked before
            if (!Pyrix64RamLoad(Ram, Address, &Values[r]))
            {
                return false;
            }

            Address += Pyrix64StackEntrySize;
        }
    }

    // all reads done
    for (uint32_t r = Pyrix64RegisterCount - 1; r >= 1; r--)
    {
        if (Mask & (1u << r))
        {
            // write registers
            Pyrix64RegWrite(Cpu, r, Values[r]);
        }
    }

    Pyrix64ActiveStackPointerWrite(Cpu, Address);

    return true;
}

//saves backjump adress to r31,
// then jumps
static bool Pyrix64InstrCall(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)Ram;
    (void)ExtWord;

    const uint64_t Target = Pyrix64DecodeAddress(Instruction);

    // if the call works r31 is written
    if (Target >= Pyrix64RamSize)
    {
        printf("::  error: CALL target out of range (0x%" PRIX64 ")\n", Target);

        return false;
    }

    Pyrix64RegWrite(Cpu, Pyrix64RegisterRa, Cpu->Pc + Pyrix64InstructionLength(Instruction));

    *NextPc = Target;

    return true;
}

//looks into r31 for return adress
// jumps to that adress while r31 is unchanged
//
// also epc is just for exceptions, so its not used
static bool Pyrix64InstrRet(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)Ram;
    (void)Instruction;
    (void)ExtWord;

    const uint64_t Target = Pyrix64RegRead(Cpu, Pyrix64RegisterRa);

    if (Target >= Pyrix64RamSize)
    {
        printf("::  error: RET target out of range (0x%016" PRIX64 ")\n", Target);

        return false;
    }

    *NextPc = Target;

    return true;
}

//compares 2 signed numbers
static bool Pyrix64SignedLess(
    uint64_t A,
    uint64_t B
){
    const uint64_t SignA = A >> 63;
    const uint64_t SignB = B >> 63;

    if (SignA != SignB)
    {
        return SignA == 1;
    }

    return A < B;
}

//rs1/2 have the compare values, then the opcode
// has the condition and we store the target adress in a extra
// word. but we only check if the branch is taken
static bool Pyrix64Branch(
    Pyrix64Cpu *Cpu,
    uint32_t Instruction,
    uint32_t ExtWord,
    bool Take,
    uint64_t *NextPc
){
    (void)Cpu;
    (void)Instruction;

    if (!Take)
    {
        // next pc stays teh same
        return true;
    }

    const uint64_t Target = ExtWord;

    if (Target >= Pyrix64RamSize)
    {
        printf("::  error: branch target out of range (0x%" PRIX64 ")\n", Target);

        return false;
    }

    *NextPc = Target;

    return true;
}

static bool Pyrix64InstrJump(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)Cpu;
    (void)Ram;
    (void)ExtWord;

    const uint64_t Target = Pyrix64DecodeAddress(Instruction);

    if (Target >= Pyrix64RamSize)
    {
        printf("::  error: JUMP target out of range (0x%" PRIX64 ")\n", Target);

        return false;
    }

    *NextPc = Target;

    return true;
}

//branch if the two registers hold the same bits
static bool Pyrix64InstrBeq(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)Ram;

    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    bool Take = Pyrix64RegRead(Cpu, Rs1) == Pyrix64RegRead(Cpu, Rs2);

    return Pyrix64Branch(Cpu, Instruction, ExtWord, Take, NextPc);
}

//branches if the two registers hold different bitd
static bool Pyrix64InstrBne(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)Ram;

    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    bool Take = Pyrix64RegRead(Cpu, Rs1) != Pyrix64RegRead(Cpu, Rs2);

    return Pyrix64Branch(Cpu, Instruction, ExtWord, Take, NextPc);
}

//branches if rs1 IS smaller as a SIGNED number
static bool Pyrix64InstrBlt(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)Ram;

    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    bool Take = Pyrix64SignedLess(Pyrix64RegRead(Cpu, Rs1), Pyrix64RegRead(Cpu, Rs2));

    return Pyrix64Branch(Cpu, Instruction, ExtWord, Take, NextPc);
}

//branches if rs1 ISNT smaller as a SIGNED number
static bool Pyrix64InstrBge(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)Ram;

    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    bool Take = !Pyrix64SignedLess(Pyrix64RegRead(Cpu, Rs1), Pyrix64RegRead(Cpu, Rs2));

    return Pyrix64Branch(Cpu, Instruction, ExtWord, Take, NextPc);
}

//branches if rs1 IS smaller as an UNSIGNED number
static bool Pyrix64InstrBltu(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)Ram;

    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    bool Take = Pyrix64RegRead(Cpu, Rs1) < Pyrix64RegRead(Cpu, Rs2);

    return Pyrix64Branch(Cpu, Instruction, ExtWord, Take, NextPc);
}

static bool Pyrix64InstrBgeu(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    (void)Ram;

    uint8_t Rs1 = Pyrix64DecodeRs1(Instruction);
    uint8_t Rs2 = Pyrix64DecodeRs2(Instruction);

    bool Take = Pyrix64RegRead(Cpu, Rs1) >= Pyrix64RegRead(Cpu, Rs2);

    return Pyrix64Branch(Cpu, Instruction, ExtWord, Take, NextPc);
}

//dispatch table
// opcode to instruction
static const Pyrix64OpFn Pyrix64DispatchTable[0x100] =
{
    [Pyrix64opNop]     = Pyrix64InstrNop,
    [Pyrix64opHalt]    = Pyrix64InstrHalt,
    [Pyrix64opMov]     = Pyrix64InstrMov,
    [Pyrix64opAdd]     = Pyrix64InstrAdd,
    [Pyrix64opSub]     = Pyrix64InstrSub,
    [Pyrix64opAnd]     = Pyrix64InstrAnd,
    [Pyrix64opOr]      = Pyrix64InstrOr,
    [Pyrix64opXor]     = Pyrix64InstrXor,
    [Pyrix64opNot]     = Pyrix64InstrNot,
    [Pyrix64opMovI]    = Pyrix64InstrMovI,
    [Pyrix64opLoad]    = Pyrix64InstrLoad,
    [Pyrix64opStore]   = Pyrix64InstrStore,
    [Pyrix64opJump]    = Pyrix64InstrJump,
    [Pyrix64opPush]    = Pyrix64InstrPush,
    [Pyrix64opPop]     = Pyrix64InstrPop,
    [Pyrix64opPushFp]  = Pyrix64InstrPushFp,
    [Pyrix64opPopFp]   = Pyrix64InstrPopFp,
    [Pyrix64opPushM]   = Pyrix64InstrPushM,
    [Pyrix64opPopM]    = Pyrix64InstrPopM,
    [Pyrix64opBeq]     = Pyrix64InstrBeq,
    [Pyrix64opBne]     = Pyrix64InstrBne,
    [Pyrix64opBlt]     = Pyrix64InstrBlt,
    [Pyrix64opBge]     = Pyrix64InstrBge,
    [Pyrix64opBltu]    = Pyrix64InstrBltu,
    [Pyrix64opBgeu]    = Pyrix64InstrBgeu,
    [Pyrix64opCall]    = Pyrix64InstrCall,
    [Pyrix64opRet]     = Pyrix64InstrRet

};

bool Pyrix64DecodeExecute(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    uint32_t Instruction,
    uint32_t ExtWord,
    uint64_t *NextPc
){
    uint8_t Opcode = Pyrix64DecodeOpcode(Instruction);

    const Pyrix64OpFn Handler = Pyrix64DispatchTable[Opcode];

    if (Handler == NULL)
    {
        printf("::  error: unknown opcode 0x%02X\n", Opcode);

        return false;
    }
    *NextPc = Cpu->Pc + Pyrix64InstructionLength(Instruction);

    return Handler(Cpu, Ram, Instruction, ExtWord, NextPc);
}
