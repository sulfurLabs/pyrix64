/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: asm.h
 * DESC.: pyx assembler core
 *
 * CREATED BY: offihito
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

#ifndef Pyrix64AsmH
#define Pyrix64AsmH 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PYX_MAX_SYMBOLS 256
#define PYX_MAX_NAME_LEN 64

typedef struct
{
    char Name[PYX_MAX_NAME_LEN];
    uint32_t Address;
} PyxSymbol;

typedef struct
{
    PyxSymbol Symbols[PYX_MAX_SYMBOLS];
    size_t Count;
} PyxSymbolTable;

bool PyxAssemble(
    const char *Source,
    uint32_t *Output,
    size_t MaxInstructions,
    size_t *OutCount
);

#endif /* Pyrix64AsmH */
