/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: formats.c
 * DESC.: instruction format
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
#include <stdlib.h>

#include "instr/formats.h"

// this file is a library for the register
// fromats, and instructions use them then

//  31-24 | 23-19 | 18-14 | 13-9
// opcode |  rd   |  rs1  |  rs2
static uint32_t PyxEncodeRegisterFormat(uint32_t Opcode, int Rd, int Rs1, int Rs2)
{
    return (
        (Opcode << 24) |
        ((uint32_t)Rd << 19) |
        ((uint32_t)Rs1 << 14) |
        ((uint32_t)Rs2 << 9)
    );
}

// 31-24  | 23-19 | 18-0
// opcode |  rd   | signed immediate
static uint32_t PyxEncodeImmediateFormat(uint32_t Opcode, int Rd, long Imm)
{
    return (
        (Opcode << 24) |
        ((uint32_t)Rd << 19) |
        ((uint32_t)Imm & ((1 << PYX_IMM_BITS) - 1))
    );
}

static bool PyxParseRegisterOperands(
    const char *File,
    int Number,
    const char **Cursor,
    int *Registers,
    int Count,
    const char *Expected
){
    char Token[PYX_TOKEN_LEN];
    int Index;

    for (Index = 0; Index < Count; Index++)
    {
        if (!PyxNextToken(Cursor, Token))
        {
            PyxError(File, Number, Expected, NULL);

            return false;
        }

        Registers[Index] = PyxParseRegister(Token);

        if (Registers[Index] < 0)
        {
            PyxError(File, Number, "Invalid register", Token);

            return false;
        }
    }

    return true;
}

//formats

bool PyxFormatRegisters3(
    const char *File,
    int Number,
    const char *Mnemonic,
    const char *Operands,
    uint32_t Opcode,
    PyxImage *Image
){
    int Registers[3];
    char Token[PYX_TOKEN_LEN];
    const char *Cursor = Operands;

    if (!PyxParseRegisterOperands(File, Number, &Cursor, Registers, 3, "Expected exactly three register operands"))
    {
        return false;
    }

    if (PyxNextToken(&Cursor, Token))
    {
        PyxError(File, Number, "Expected exactly three register operands", NULL);

        return false;
    }

    if (!PyxImageAppendWord(
            Image,
            PyxEncodeRegisterFormat(
                Opcode,
                Registers[0],
                Registers[1],
                Registers[2]
            )
        )
    ) return false;

    printf(
        "::  info: Parsed %s: rd=r%d, rs1=r%d, rs2=r%d\n",
        Mnemonic,
        Registers[0],
        Registers[1],
        Registers[2]
    );

    return true;
}

bool PyxFormatRegisters2(
    const char *File,
    int Number,
    const char *Mnemonic,
    const char *Operands,
    uint32_t Opcode,
    PyxImage *Image
){
    int Registers[2];
    char Token[PYX_TOKEN_LEN];
    const char *Cursor = Operands;

    if (!PyxParseRegisterOperands(File, Number, &Cursor, Registers, 2, "Expected a register and source operand"))
    {
        return false;
    }

    if (PyxNextToken(&Cursor, Token))
    {
        PyxError(File, Number, "Expected exactly two register operands", NULL);

        return false;
    }

    if (!PyxImageAppendWord(
            Image,
            PyxEncodeRegisterFormat(
                Opcode,
                Registers[0],
                Registers[1],
                0
            )
        )
    ) return false;

    printf(
        "::  info: Parsed %s: rd=r%d, rs=r%d\n",
        Mnemonic,
        Registers[0],
        Registers[1]
    );

    return true;
}

bool PyxFormatImmediate(
    const char *File,
    int Number,
    const char *Mnemonic,
    const char *Operands,
    uint32_t Opcode,
    PyxImage *Image
){
    char Token[PYX_TOKEN_LEN];
    char *End;
    const char *Cursor = Operands;
    int Rd;
    long Value;

    //destination register
    if (!PyxNextToken(&Cursor, Token))
    {
        PyxError(File, Number, "Expected a register operand", NULL);

        return false;
    }

    Rd = PyxParseRegister(Token);

    if (Rd < 0)
    {
        PyxError(File, Number, "Invalid register", Token);

        return false;
    }

    //immediate value
    if (!PyxNextToken(&Cursor, Token))
    {
        PyxError(File, Number, "Expected an immediate operand", NULL);

        return false;
    }

    Value = strtol(Token, &End, 10);

    if (
        End == Token        ||
        *End != '\0'        ||
        Value < PYX_IMM_MIN ||
        Value > PYX_IMM_MAX
    ){
        PyxError(File, Number, "Invalid immediate", Token);

        return false;
    }

    if (PyxNextToken(&Cursor, Token))
    {
        PyxError(File, Number, "Expected exactly register and immediate operands", NULL);

        return false;
    }

    if (
        !PyxImageAppendWord
        (
            Image,
            PyxEncodeImmediateFormat(Opcode, Rd, Value)
        )
    ) return false;

    printf(
        "::  info: Parsed %s: rd=r%d, imm=%ld\n",
        Mnemonic,
        Rd,
        Value
    );

    return true;
}
