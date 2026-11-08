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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "pyrix/isa.h"
#include "internal.h"

static bool PyxStrCaseEq(
    const char *Left,
    const char *Right
){
    while (
        *Left != '\0' &&
        tolower((unsigned char)*Left) == tolower((unsigned char)*Right)
    ){
        Left++;
        Right++;
    }

    return *Left == '\0' && *Right == '\0';
}
static int PyxParseRegister(const char *Token)
{
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

static bool PyxNextToken(
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

//error is with file and line number
static void PyxError(
    const char *File,
    int Number,
    const char *Message,
    const char *Token
){
    fprintf(stderr, "Error: %s:%d: %s", File, Number, Message);

    if (Token != NULL)
    {
        fprintf(stderr, " '%s'", Token);
    }

    fputc('\n', stderr);
}

static uint32_t PyxEncode(uint32_t Opcode, int Rd, int Rs1, int Rs2)
{
    return (
        (Opcode << 24) |
        ((uint32_t)Rd << 19) |
        ((uint32_t)Rs1 << 14) |
        ((uint32_t)Rs2 << 9)
    );
}

static bool PyxParseLine(const char *File, char *Line, int Number, PyxImage *Image)
{
    const char *Cursor = Line;

    uint32_t Opcode;
    char Token[PYX_TOKEN_LEN];
    int Registers[PYX_OPERAND_COUNT];
    int Count = 0;

    if (!PyxNextToken(&Cursor, Token))
    {
        return true; //empty line
    }

    //dw it will get better
    if (PyxStrCaseEq(Token, "ADD") || PyxStrCaseEq(Token, "SUB"))
    {
        Opcode = (Token[0] == 'a' || Token[0] == 'A') ? Pyrix64opAdd : Pyrix64opSub;
    }
    else
    {
        PyxError(File, Number, "Unknown instruction", Token);
        return false;
    }

    while (PyxNextToken(&Cursor, Token))
    {
        if (Count >= PYX_OPERAND_COUNT)
        {
            PyxError(File, Number, "Expected exactly three register operands", NULL);
            return false;
        }

        Registers[Count++] = PyxParseRegister(Token);

        if (Registers[Count - 1] < 0)
        {
            PyxError(File, Number, "Invalid register", Token);
            return false;
        }
    }

    if (Count != PYX_OPERAND_COUNT)
    {
        PyxError(File, Number, "Expected exactly three register operands", NULL);
        return false;
    }
    if (!PyxImageAppendWord(
            Image,
            PyxEncode(
                Opcode,
                Registers[0],
                Registers[1],
                Registers[2]
            )
        )
    ) return false;

    printf(
        "Parsed %s: rd=r%d, rs1=r%d, rs2=r%d\n",
        (Opcode == Pyrix64opAdd) ? "ADD" : "SUB",
        Registers[0],
        Registers[1],
        Registers[2]
    );

    return true;
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
