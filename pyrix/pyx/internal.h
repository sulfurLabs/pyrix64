/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: internal.h
 * DESC.: shared pyx internals (image and parser interface)
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

#ifndef p64internalh
#define p64internalh 1

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

typedef struct
{
    const char *Name;
    uint32_t Opcode;
} PyxInstruction;

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

#endif /*p64internalh */
