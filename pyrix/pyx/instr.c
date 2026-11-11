/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: instr.c
 * DESC.: pyx assembler instruction table and dispatch
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
#include "instr/instr.h"

//one instruction definition:
// its name and implementation
typedef struct
{
    const char *Name;
    PyxInstrFn *Run;
} PyxInstruction;

//instructions are always uppercase in pyx
static const PyxInstruction PyxInstructionTable[] =
{
    { "ADD", PyxInstrAdd },
    { "SUB", PyxInstrSub },
    { "MOV", PyxInstrMov },
    { "NOP", PyxInstrNop },
    { "HALT", PyxInstrHalt },
    { "MOVI", PyxInstrMovi },
    { "AND", PyxInstrAnd },
    { "OR", PyxInstrOr }
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

bool PyxProcessInstruction(
    const char *File,
    int LineNumber,
    const char *Mnemonic,
    const char *Operands,
    PyxImage *Image
){
    const PyxInstruction *Instruction = PyxLookupInstruction(Mnemonic);

    if (Instruction == NULL)
    {
        PyxError(File, LineNumber, "Unknown instruction", Mnemonic);
        return false;
    }

    return Instruction->Run(File, LineNumber, Operands, Image);
}
