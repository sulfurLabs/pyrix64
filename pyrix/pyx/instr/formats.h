/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: formats.h
 * DESC.: instruction formats
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

#ifndef pyx64FormatsH
#define pyx64FormatsH 1

#include <stdbool.h>
#include <stdint.h>

#include "internal.h"

#define PYX_IMM_BITS 19
#define PYX_IMM_MAX ((1 << (PYX_IMM_BITS - 1)) - 1)
#define PYX_IMM_MIN (-(1 << (PYX_IMM_BITS - 1)))

//opcode | rd | rs1 | rs2
bool PyxFormatRegisters3(
    const char *File,
    int Number,
    const char *Mnemonic,
    const char *Operands,
    uint32_t Opcode,
    PyxImage *Image
);

//opcode | rd | rs1
bool PyxFormatRegisters2(
    const char *File,
    int Number,
    const char *Mnemonic,
    const char *Operands,
    uint32_t Opcode,
    PyxImage *Image
);

// opcode | rd | signed
bool PyxFormatImmediate(
    const char *File,
    int Number,
    const char *Mnemonic,
    const char *Operands,
    uint32_t Opcode,
    PyxImage *Image
);

#endif /*pyx64FormatsH*/
