/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: operand.c
 * DESC.: pyx assembler shared operand and token helpers
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
#include <stdlib.h>
#include <stdbool.h>

#include "internal.h"

bool PyxNextToken(
    const char **Cursor,
    char *Token
){
    const char *Start;
    size_t Index = 0;

    while (
        **Cursor != '\0' &&
        (
            isspace((unsigned char)**Cursor) ||
            **Cursor == ','
        )
    ){
        (*Cursor)++;
    }

    Start = *Cursor;
    while (**Cursor != '\0' && !isspace((unsigned char)**Cursor) && **Cursor != ',')
    {
        if (Index + 1 < PYX_TOKEN_LEN)
        {
            Token[Index] = **Cursor;
            Index++;
        }

        (*Cursor)++;
    }

    Token[Index] = '\0';

    return Start != *Cursor;
}

//accepts r0 through r3
int PyxParseRegister(
    const char *Token
){
    char *End;
    long Number;

    if (
        (
            Token[0] != 'r' &&
            Token[0] != 'R'
        ) ||
        Token[1] == '\0'
    ){
        return -1;
    }

    Number = strtol(Token + 1, &End, 10);

    if (
        End == Token + 1 ||
        *End != '\0'     ||
        Number < 0       ||
        Number > 31
    ){
        return -1;
    }

    return (int)Number;
}
