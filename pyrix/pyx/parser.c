/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: parser.c
 * DESC.: pyx assembler parser file
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
#include <string.h>
#include <stdbool.h>

#include "internal.h"

static bool PyxParseLine(const char *File, char *Line, int Number, PyxImage *Image)
{
    const char *Cursor = Line;

    //const PyxInstruction *Instruction;
    char Token[PYX_TOKEN_LEN];

    if (!PyxNextToken(&Cursor, Token))
    {
        return true; //empty line
    }

    // rest of the line is still operand text
    return PyxProcessInstruction(File, Number, Token, Cursor, Image);
}

bool PyxParseFile(
    const char *Path,
    PyxImage *Image
){
    FILE *Input;
    char Line[PYX_LINE_LEN];
    char *Comment;
    int Number = 0;

    Input = fopen(Path, "r");

    if (Input == NULL)
    {
        fprintf(stderr, "::  error: cannot open '%s'\n", Path);

        return false;
    }

    while (fgets(Line, sizeof(Line), Input) != NULL)
    {
        Number++;
        Comment = strchr(Line, ';');

        if (Comment != NULL)
        {
            *Comment = '\0';
        }
        if (!PyxParseLine(Path, Line, Number, Image))
        {
            fclose(Input);

            return false;
        }
    }

    fclose(Input);

    return true;
}
