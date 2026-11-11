/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: movi.c
 * DESC.: MOVI instr
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

//MOVI r1, 42
// loads signed 19bit immediate
bool PyxInstrMovi(
    const char *File,
    int LineNumber,
    const char *Operands,
    PyxImage *Image
){
    return PyxFormatImmediate(
        File,
        LineNumber,
        "MOVI",
        Operands,
        Pyrix64opMovI,
        Image
    );
}
