/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: asm.c
 * DESC.: pyx assembler implementation
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

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "asm.h"
#include "pyrix/enc.h"
#include "pyrix/isa.h"

static void PyxStripComment(char *Line)
{
    for (size_t i = 0; Line[i] != '\0'; i++)
    {
        if (Line[i] == ';' || Line[i] == '#')
        {
            Line[i] = '\0';
            break;
        }
    }
}

static char *PyxTrim(char *Str)
{
    while (isspace((unsigned char)*Str))
    {
        Str++;
    }

    if (*Str == '\0')
    {
        return Str;
    }

    char *End = Str + strlen(Str) - 1;

    while (End > Str && isspace((unsigned char)*End))
    {
        *End = '\0';
        End--;
    }

    return Str;
}

static size_t PyxParseStringLiteral(const char *Str, char *OutBuf, size_t MaxLen)
{
    const char *Start = strchr(Str, '"');

    if (Start == NULL)
    {
        return 0;
    }

    Start++;
    size_t Len = 0;

    while (*Start != '\0' && *Start != '"' && Len + 1 < MaxLen)
    {
        if (*Start == '\\' && *(Start + 1) != '\0')
        {
            Start++;
            if (*Start == 'n') OutBuf[Len++] = '\n';
            else if (*Start == 't') OutBuf[Len++] = '\t';
            else if (*Start == 'r') OutBuf[Len++] = '\r';
            else if (*Start == '0') OutBuf[Len++] = '\0';
            else if (*Start == '\\') OutBuf[Len++] = '\\';
            else if (*Start == '"') OutBuf[Len++] = '"';
            else OutBuf[Len++] = *Start;
            Start++;
        }
        else
        {
            OutBuf[Len++] = *Start++;
        }
    }

    OutBuf[Len] = '\0';

    return Len;
}

static bool PyxSymbolFind(
    const PyxSymbolTable *Table,
    const char *Name,
    uint32_t *Address
){
    for (size_t i = 0; i < Table->Count; i++)
    {
        if (strcmp(Table->Symbols[i].Name, Name) == 0)
        {
            if (Address != NULL)
            {
                *Address = Table->Symbols[i].Address;
            }

            return true;
        }
    }

    return false;
}

static bool PyxSymbolAdd(
    PyxSymbolTable *Table,
    const char *Name,
    uint32_t Address
){
    if (Table->Count >= PYX_MAX_SYMBOLS)
    {
        printf(":: error: symbol table full\n");

        return false;
    }

    if (PyxSymbolFind(Table, Name, NULL))
    {
        printf(":: error: duplicate symbol '%s'\n", Name);

        return false;
    }

    strncpy(Table->Symbols[Table->Count].Name, Name, PYX_MAX_NAME_LEN - 1);
    Table->Symbols[Table->Count].Name[PYX_MAX_NAME_LEN - 1] = '\0';
    Table->Symbols[Table->Count].Address = Address;
    Table->Count++;

    return true;
}

static bool PyxParseReg(const char *Token, uint8_t *Reg)
{
    if (strcasecmp(Token, "zero") == 0)
    {
        *Reg = 0;

        return true;
    }

    if (strcasecmp(Token, "ra") == 0)
    {
        *Reg = 31;

        return true;
    }

    if (Token[0] == 'r' || Token[0] == 'R')
    {
        char *End = NULL;
        long Val = strtol(Token + 1, &End, 10);

        if (End != Token + 1 && *End == '\0' && Val >= 0 && Val < 32)
        {
            *Reg = (uint8_t)Val;

            return true;
        }
    }

    return false;
}

static bool PyxParseImmediate(
    const char *Token,
    const PyxSymbolTable *Table,
    int64_t *Value
){
    uint32_t Addr = 0;

    if (PyxSymbolFind(Table, Token, &Addr))
    {
        *Value = (int64_t)Addr;

        return true;
    }

    char *End = NULL;
    long long Val = strtoll(Token, &End, 0);

    if (End != Token && *End == '\0')
    {
        *Value = (int64_t)Val;

        return true;
    }

    return false;
}

static size_t PyxTokenize(char *Line, char *Tokens[], size_t MaxTokens)
{
    size_t Count = 0;
    char *SavePtr = NULL;
    char *Token = strtok_r(Line, " \t,", &SavePtr);

    while (Token != NULL && Count < MaxTokens)
    {
        Tokens[Count++] = Token;
        Token = strtok_r(NULL, " \t,", &SavePtr);
    }

    return Count;
}

bool PyxAssemble(
    const char *Source,
    uint32_t *Output,
    size_t MaxInstructions,
    size_t *OutCount
){
    PyxSymbolTable Table;
    memset(&Table, 0, sizeof(Table));

    // pass 1: collect labels and count instructions
    char *SourceCopy = strdup(Source);

    if (SourceCopy == NULL)
    {
        return false;
    }

    uint32_t CurrentAddress = 0;
    char *Line = strtok(SourceCopy, "\n\r");
    size_t LineNumber = 0;

    while (Line != NULL)
    {
        LineNumber++;
        char LineBuf[512];
        strncpy(LineBuf, Line, sizeof(LineBuf) - 1);
        LineBuf[sizeof(LineBuf) - 1] = '\0';

        PyxStripComment(LineBuf);
        char *Trimmed = PyxTrim(LineBuf);

        if (*Trimmed != '\0')
        {
            char *Colon = strchr(Trimmed, ':');

            if (Colon != NULL)
            {
                *Colon = '\0';
                char *Label = PyxTrim(Trimmed);
                char *AfterColon = PyxTrim(Colon + 1);

                if (*Label == '\0')
                {
                    printf(":: error on line %zu: empty label\n", LineNumber);
                    free(SourceCopy);

                    return false;
                }

                bool IsData = false;

                if (*AfterColon != '\0')
                {
                    if (*AfterColon == '.')
                    {
                        IsData = true;
                    }
                }
                else
                {
                    const char *Peek = Line + strlen(Line) + 1;

                    while (*Peek != '\0')
                    {
                        while (*Peek == '\r' || *Peek == '\n' || isspace((unsigned char)*Peek))
                        {
                            Peek++;
                        }

                        if (*Peek == ';' || *Peek == '#')
                        {
                            while (*Peek != '\0' && *Peek != '\n' && *Peek != '\r')
                            {
                                Peek++;
                            }
                            continue;
                        }

                        if (*Peek == '.')
                        {
                            IsData = true;
                        }

                        break;
                    }
                }

                uint32_t SymbolAddr = IsData ? (CurrentAddress * 4) : CurrentAddress;

                if (!PyxSymbolAdd(&Table, Label, SymbolAddr))
                {
                    printf(":: error on line %zu: failed to add symbol '%s'\n", LineNumber, Label);
                    free(SourceCopy);

                    return false;
                }

                Trimmed = AfterColon;
            }

            if (*Trimmed != '\0')
            {
                if (strncasecmp(Trimmed, ".string", 7) == 0 || strncasecmp(Trimmed, ".asciz", 6) == 0)
                {
                    char StrBuf[512];
                    size_t StrLen = PyxParseStringLiteral(Trimmed, StrBuf, sizeof(StrBuf));
                    size_t Words = (StrLen + 1 + 3) / 4;
                    CurrentAddress += (uint32_t)Words;
                }
                else
                {
                    CurrentAddress++;
                }
            }
        }

        Line = strtok(NULL, "\n\r");
    }

    free(SourceCopy);

    // pass 2: encode instructions
    SourceCopy = strdup(Source);

    if (SourceCopy == NULL)
    {
        return false;
    }

    size_t InstrCount = 0;
    Line = strtok(SourceCopy, "\n\r");
    LineNumber = 0;

    while (Line != NULL)
    {
        LineNumber++;
        char LineBuf[512];
        strncpy(LineBuf, Line, sizeof(LineBuf) - 1);
        LineBuf[sizeof(LineBuf) - 1] = '\0';

        PyxStripComment(LineBuf);
        char *Trimmed = PyxTrim(LineBuf);

        if (*Trimmed != '\0')
        {
            char *Colon = strchr(Trimmed, ':');

            if (Colon != NULL)
            {
                Trimmed = PyxTrim(Colon + 1);
            }

            if (*Trimmed != '\0')
            {
                if (strncasecmp(Trimmed, ".string", 7) == 0 || strncasecmp(Trimmed, ".asciz", 6) == 0)
                {
                    char StrBuf[512];
                    size_t StrLen = PyxParseStringLiteral(Trimmed, StrBuf, sizeof(StrBuf));
                    size_t TotalBytes = StrLen + 1;
                    size_t Words = (TotalBytes + 3) / 4;

                    for (size_t w = 0; w < Words; w++)
                    {
                        uint32_t Word = 0;

                        for (int b = 0; b < 4; b++)
                        {
                            size_t ByteIdx = w * 4 + b;
                            uint8_t ByteVal = (ByteIdx < TotalBytes) ? (uint8_t)StrBuf[ByteIdx] : 0;
                            Word |= ((uint32_t)ByteVal << (b * 8));
                        }

                        if (InstrCount >= MaxInstructions)
                        {
                            printf(":: error: maximum instruction capacity exceeded\n");
                            free(SourceCopy);

                            return false;
                        }

                        Output[InstrCount++] = Word;
                    }

                    Line = strtok(NULL, "\n\r");
                    continue;
                }

                if (InstrCount >= MaxInstructions)
                {
                    printf(":: error: maximum instruction capacity exceeded\n");
                    free(SourceCopy);

                    return false;
                }

                char *Tokens[8];
                size_t NumTokens = PyxTokenize(Trimmed, Tokens, 8);

                if (NumTokens == 0)
                {
                    Line = strtok(NULL, "\n\r");
                    continue;
                }

                const char *Op = Tokens[0];
                uint32_t Encoded = 0;

                if (strcmp(Op, "NOP") == 0)
                {
                    Encoded = Pyrix64EncodeI(Pyrix64opNop, 0, 0);
                }
                else if (strcmp(Op, "HALT") == 0)
                {
                    Encoded = Pyrix64EncodeI(Pyrix64opHalt, 0, 0);
                }
                else if (strcmp(Op, "RET") == 0)
                {
                    Encoded = Pyrix64EncodeI(Pyrix64opRet, 0, 0);
                }
                else if (strcmp(Op, "PUSH") == 0)
                {
                    uint8_t Reg = 0;

                    if (NumTokens < 2 || !PyxParseReg(Tokens[1], &Reg))
                    {
                        printf(":: error on line %zu: invalid PUSH operand\n", LineNumber);
                        free(SourceCopy);

                        return false;
                    }

                    Encoded = Pyrix64EncodeI(Pyrix64opPush, Reg, 0);
                }
                else if (strcmp(Op, "POP") == 0)
                {
                    uint8_t Reg = 0;

                    if (NumTokens < 2 || !PyxParseReg(Tokens[1], &Reg))
                    {
                        printf(":: error on line %zu: invalid POP operand\n", LineNumber);
                        free(SourceCopy);

                        return false;
                    }

                    Encoded = Pyrix64EncodeI(Pyrix64opPop, Reg, 0);
                }
                else if (strcmp(Op, "JMP") == 0)
                {
                    int64_t Target = 0;

                    if (NumTokens < 2 || !PyxParseImmediate(Tokens[1], &Table, &Target))
                    {
                        printf(":: error on line %zu: invalid JMP target '%s'\n", LineNumber, NumTokens >= 2 ? Tokens[1] : "");
                        free(SourceCopy);

                        return false;
                    }

                    Encoded = Pyrix64EncodeJ(Pyrix64opJmp, (uint32_t)Target);
                }
                else if (strcmp(Op, "CALL") == 0)
                {
                    int64_t Target = 0;

                    if (NumTokens < 2 || !PyxParseImmediate(Tokens[1], &Table, &Target))
                    {
                        printf(":: error on line %zu: invalid CALL target '%s'\n", LineNumber, NumTokens >= 2 ? Tokens[1] : "");
                        free(SourceCopy);

                        return false;
                    }

                    Encoded = Pyrix64EncodeJ(Pyrix64opCall, (uint32_t)Target);
                }
                else if (strcmp(Op, "MOVI") == 0)
                {
                    uint8_t Rd = 0;
                    int64_t Imm = 0;

                    if (NumTokens < 3 || !PyxParseReg(Tokens[1], &Rd) || !PyxParseImmediate(Tokens[2], &Table, &Imm))
                    {
                        printf(":: error on line %zu: invalid MOVI operands\n", LineNumber);
                        free(SourceCopy);

                        return false;
                    }

                    Encoded = Pyrix64EncodeI(Pyrix64opMovI, Rd, (uint32_t)Imm);
                }
                else if (strcmp(Op, "MOV") == 0)
                {
                    uint8_t Rd = 0;
                    uint8_t Rs1 = 0;

                    if (NumTokens < 3 || !PyxParseReg(Tokens[1], &Rd) || !PyxParseReg(Tokens[2], &Rs1))
                    {
                        printf(":: error on line %zu: invalid MOV operands\n", LineNumber);
                        free(SourceCopy);

                        return false;
                    }

                    Encoded = Pyrix64EncodeR(Pyrix64opMov, Rd, Rs1, 0);
                }
                else if (strcmp(Op, "NOT") == 0)
                {
                    uint8_t Rd = 0;
                    uint8_t Rs1 = 0;

                    if (NumTokens < 3 || !PyxParseReg(Tokens[1], &Rd) || !PyxParseReg(Tokens[2], &Rs1))
                    {
                        printf(":: error on line %zu: invalid NOT operands\n", LineNumber);
                        free(SourceCopy);

                        return false;
                    }

                    Encoded = Pyrix64EncodeR(Pyrix64opNot, Rd, Rs1, 0);
                }
                else if (strcmp(Op, "LOAD") == 0 || strcmp(Op, "LOADB") == 0)
                {
                    uint8_t Rd = 0;
                    uint8_t Opcode = (strcmp(Op, "LOADB") == 0) ? Pyrix64opLoadB : Pyrix64opLoad;

                    if (NumTokens < 3 || !PyxParseReg(Tokens[1], &Rd))
                    {
                        printf(":: error on line %zu: invalid %s destination\n", LineNumber, Op);
                        free(SourceCopy);

                        return false;
                    }

                    uint8_t Rs1 = 0;
                    int64_t Addr = 0;

                    if (PyxParseReg(Tokens[2], &Rs1))
                    {
                        Encoded = Pyrix64EncodeR(Opcode, Rd, Rs1, 0);
                    }
                    else if (PyxParseImmediate(Tokens[2], &Table, &Addr))
                    {
                        Encoded = Pyrix64EncodeI(Opcode, Rd, (uint32_t)Addr);
                    }
                    else
                    {
                        printf(":: error on line %zu: invalid %s address operand '%s'\n", LineNumber, Op, Tokens[2]);
                        free(SourceCopy);

                        return false;
                    }
                }
                else if (strcmp(Op, "STORE") == 0 || strcmp(Op, "STOREB") == 0)
                {
                    uint8_t Rs = 0;
                    uint8_t Opcode = (strcmp(Op, "STOREB") == 0) ? Pyrix64opStoreB : Pyrix64opStore;

                    if (NumTokens < 3 || !PyxParseReg(Tokens[1], &Rs))
                    {
                        printf(":: error on line %zu: invalid %s source\n", LineNumber, Op);
                        free(SourceCopy);

                        return false;
                    }

                    uint8_t RsAddr = 0;
                    int64_t Addr = 0;

                    if (PyxParseReg(Tokens[2], &RsAddr))
                    {
                        Encoded = Pyrix64EncodeR(Opcode, Rs, RsAddr, 0);
                    }
                    else if (PyxParseImmediate(Tokens[2], &Table, &Addr))
                    {
                        Encoded = Pyrix64EncodeI(Opcode, Rs, (uint32_t)Addr);
                    }
                    else
                    {
                        printf(":: error on line %zu: invalid %s address operand '%s'\n", LineNumber, Op, Tokens[2]);
                        free(SourceCopy);

                        return false;
                    }
                }
                else if (strcmp(Op, "ADD") == 0 ||
                         strcmp(Op, "SUB") == 0 ||
                         strcmp(Op, "AND") == 0 ||
                         strcmp(Op, "OR")  == 0 ||
                         strcmp(Op, "XOR") == 0)
                {
                    uint8_t Rd = 0;
                    uint8_t Rs1 = 0;
                    uint8_t Rs2 = 0;

                    if (NumTokens < 4 || !PyxParseReg(Tokens[1], &Rd) || !PyxParseReg(Tokens[2], &Rs1) || !PyxParseReg(Tokens[3], &Rs2))
                    {
                        printf(":: error on line %zu: invalid operands for %s\n", LineNumber, Op);
                        free(SourceCopy);

                        return false;
                    }

                    uint8_t Opcode = 0;
                    if (strcmp(Op, "ADD") == 0) Opcode = Pyrix64opAdd;
                    else if (strcmp(Op, "SUB") == 0) Opcode = Pyrix64opSub;
                    else if (strcmp(Op, "AND") == 0) Opcode = Pyrix64opAnd;
                    else if (strcmp(Op, "OR")  == 0) Opcode = Pyrix64opOr;
                    else if (strcmp(Op, "XOR") == 0) Opcode = Pyrix64opXor;

                    Encoded = Pyrix64EncodeR(Opcode, Rd, Rs1, Rs2);
                }
                else if (strcmp(Op, "BEQ") == 0 || strcmp(Op, "BNE") == 0)
                {
                    uint8_t Rs1 = 0;
                    uint8_t Rs2 = 0;
                    int64_t Target = 0;

                    if (NumTokens < 4 || !PyxParseReg(Tokens[1], &Rs1) || !PyxParseReg(Tokens[2], &Rs2) || !PyxParseImmediate(Tokens[3], &Table, &Target))
                    {
                        printf(":: error on line %zu: invalid branch operands for %s\n", LineNumber, Op);
                        free(SourceCopy);

                        return false;
                    }

                    uint8_t Opcode = (strcmp(Op, "BEQ") == 0) ? Pyrix64opBeq : Pyrix64opBne;
                    Encoded = Pyrix64EncodeB(Opcode, Rs1, Rs2, (uint32_t)Target);
                }
                else
                {
                    printf(":: error on line %zu: unknown opcode '%s'\n", LineNumber, Op);
                    free(SourceCopy);

                    return false;
                }

                Output[InstrCount++] = Encoded;
            }
        }

        Line = strtok(NULL, "\n\r");
    }

    free(SourceCopy);
    *OutCount = InstrCount;

    return true;
}
