/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: main.c
 * DESC.: pyx assembler
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

#include "internal.h"

int main(int argc, char **argv)
{
    PyxImage Image;
    bool Ok;

    if (argc != 3)
    {
        fprintf(stderr, "usage: pyx input.pyx output.bin\n");
        return 1;
    }

    PyxImageInit(&Image);

    Ok = PyxParseFile(argv[1], &Image);

    if (Ok) Ok = PyxImageWrite(argv[2], &Image);

    PyxImageFree(&Image);

    return Ok ? 0 : 1;
}
