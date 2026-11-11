/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: mov.c
 * DESC.: MOV
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

#include "internal.h"
#include "instr/formats.h"
#include "instr/instr.h"
#include "pyrix/isa.h"

//MOV has 2 options for formats
// MOV rd, rS        - direct copy from register to another
// MOV rd, immediate - immediatly loads a value to register
bool PyxInstrMov(
    const char *File,
    int LineNumber,
    const char *Operands,
    PyxImage *Image
){
    char Token[PYX_TOKEN_LEN];
    const char *Cursor = Operands;

    if (!PyxNextToken(&Cursor, Token))
    {
        PyxError(File, LineNumber, "Expected a register operand", NULL);

        return false;
    }
    if (PyxParseRegister(Token) < 0)
    {
        PyxError(File, LineNumber, "Invalid register", Token);

        return false;
    }
    if (!PyxNextToken(&Cursor, Token))
    {
        PyxError(File, LineNumber, "Expected a register or immediate operand", NULL);

        return false;
    }

    //rs (register source) has the register format
    // the other option would then be immediate format
    if (PyxParseRegister(Token) >= 0)
    {
        return PyxFormatRegisters2(
            File,
            LineNumber,
            "MOV",
            Operands,
            Pyrix64opMov,
            Image
        );
    }

    return PyxFormatImmediate(
        File,
        LineNumber,
        "MOV",
        Operands,
        Pyrix64opMovI,
        Image
    );
}
