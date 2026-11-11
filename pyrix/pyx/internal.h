/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: internal.h
 * DESC.: header for all files in this directory, or whatever
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

#ifndef pyx64InternalH
#define pyx64InternalH 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cfg.h"

typedef struct
{
    uint8_t *Data;
    size_t Size;
    size_t Capacity;
} PyxImage;

//format/bin.c
void PyxImageInit(PyxImage *Image);
bool PyxImageAppendWord(PyxImage *Image, uint32_t Word);
bool PyxImageWrite(const char *Path, const PyxImage *Image);
void PyxImageFree(PyxImage *Image);

//parsing
bool PyxParseFile(
    const char *Path,
    PyxImage *Image
);

//instr.c
bool PyxProcessInstruction(
    const char *File,
    int LineNumber,
    const char *Mnemonic,
    const char *Operands,
    PyxImage *Image
);

//operand.c
bool PyxNextToken(
    const char **Cursor,
    char *Token
);
int PyxParseRegister(
    const char *Token
);

//error.c
void PyxError(
    const char *File,
    int Number,
    const char *Message,
    const char *Token
);

#endif /*pyx64InternalH */
