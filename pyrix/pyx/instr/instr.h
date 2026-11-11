/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: instr.h
 * DESC.: instruction main header, for all instructions
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

#ifndef pyx64InstrH
#define pyx64InstrH 1

#include <stdbool.h>

#include "internal.h"

typedef bool PyxInstrFn(
    const char *File,
    int LineNumber,
    const char *Operands,
    PyxImage *Image
);

bool PyxInstrAdd(
    const char *File,
    int LineNumber,
    const char *Operands,
    PyxImage *Image
);
bool PyxInstrSub(
    const char *File,
    int LineNumber,
    const char *Operands,
    PyxImage *Image
);
bool PyxInstrMov(
    const char *File,
    int LineNumber,
    const char *Operands,
    PyxImage *Image
);

#endif /*pyx64InstrH */
