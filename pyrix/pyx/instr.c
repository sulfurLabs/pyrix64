/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: instr.c
 * DESC.: pyx assembler instruction handling file
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

#include <ctype.h>
#include <stdio.h>

#include "pyrix/isa.h"
#include "internal.h"

//one instruction definition: its name and opcode
typedef struct
{
    const char *Name;
    uint32_t Opcode;
} PyxInstruction;

//instructions are always uppercase in pyx
static const PyxInstruction PyxInstructionTable[] =
{
    { "ADD", Pyrix64opAdd },
    { "SUB", Pyrix64opSub }
};

static bool PyxStrCaseEq(
    const char *Left,
    const char *Right
){
    while (
        *Left != '\0' &&
        tolower((unsigned char)*Left) == tolower((unsigned char)*Right)
    ){
        Left++;
        Right++;
    }

    return
        *Left == '\0' &&
        *Right == '\0'
    ;
}

static const PyxInstruction *PyxLookupInstruction(
    const char *Name
){
    size_t Index;

    //searches by name
    for (
        Index = 0;
        Index < sizeof(PyxInstructionTable) / sizeof(PyxInstructionTable[0]);
        Index++
    ){
        if (PyxStrCaseEq(Name, PyxInstructionTable[Index].Name))
        {
            return &PyxInstructionTable[Index];
        }
    }

    // unknown name
    return NULL;
}

static uint32_t PyxEncode(uint32_t Opcode, int Rd, int Rs1, int Rs2)
{
    return (
        (Opcode << 24) |
        ((uint32_t)Rd << 19) |
        ((uint32_t)Rs1 << 14) |
        ((uint32_t)Rs2 << 9)
    );
}

bool PyxProcessInstruction(
    const char *File,
    int LineNumber,
    const char *Mnemonic,
    const char *Operands,
    PyxImage *Image
){
    const PyxInstruction *Instruction = PyxLookupInstruction(Mnemonic);
    const char *Cursor = Operands;
    char Token[PYX_TOKEN_LEN];
    int Registers[PYX_OPERAND_COUNT];
    int Count = 0;

    if (Instruction == NULL)
    {
        PyxError(File, LineNumber, "Unknown instruction", Mnemonic);
        return false;
    }

    while (PyxNextToken(&Cursor, Token))
    {
        if (Count >= PYX_OPERAND_COUNT)
        {
            PyxError(File, LineNumber, "Expected exactly three register operands", NULL);
            return false;
        }

        Registers[Count++] = PyxParseRegister(Token);

        if (Registers[Count - 1] < 0)
        {
            PyxError(File, LineNumber, "Invalid register", Token);
            return false;
        }
    }

    if (Count != PYX_OPERAND_COUNT)
    {
        PyxError(File, LineNumber, "Expected exactly three register operands", NULL);
        return false;
    }
    if (!PyxImageAppendWord(
            Image,
            PyxEncode(
                Instruction->Opcode,
                Registers[0],
                Registers[1],
                Registers[2]
            )
        )
    ) return false;

    printf(
        "Parsed %s: rd=r%d, rs1=r%d, rs2=r%d\n",
        Instruction->Name,
        Registers[0],
        Registers[1],
        Registers[2]
    );

    return true;
}
