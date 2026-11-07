/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: main.c
 * DESC.: pyx assembler
 *
 * CREATED BY: emex
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "cfg.h"
#include "pyrix/enc.h"
#include "pyrix/isa.h"

static bool PyxRunTests(void)
{
    printf("::  running assembler self-tests\n");

    const char *Sample =
        "; sample loop\n"
        "MOVI r1, 10\n"
        "MOVI r2, 0\n"
        "MOVI r3, 1\n"
        "loop:\n"
        "    ADD r2, r2, r1\n"
        "    SUB r1, r1, r3\n"
        "    BNE r1, zero, loop\n"
        "HALT\n";

    uint32_t Code[64];
    size_t Count = 0;

    if (!PyxAssemble(Sample, Code, 64, &Count))
    {
        printf(" - Basic assembly test: FAILED\n");

        return false;
    }

    if (Count != 7)
    {
        printf(" - Instruction count test: FAILED\n");

        return false;
    }

    // verify loop target resolved to address 3
    uint32_t ExpectedBne = Pyrix64EncodeB(Pyrix64opBne, 1, 0, 3);

    if (Code[5] != ExpectedBne)
    {
        printf(" - Label resolution test: FAILED\n");

        return false;
    }

    printf(" - Basic assembly & label resolution: OK\n");
    printf("==> All pyx tests passed\n\n");

    return true;
}

static char *PyxReadFile(const char *Path)
{
    FILE *Fp = fopen(Path, "rb");

    if (Fp == NULL)
    {
        printf(":: error: unable to open file '%s'\n", Path);

        return NULL;
    }

    fseek(Fp, 0, SEEK_END);
    long Size = ftell(Fp);
    fseek(Fp, 0, SEEK_SET);

    if (Size < 0)
    {
        fclose(Fp);

        return NULL;
    }

    char *Buffer = malloc((size_t)Size + 1);

    if (Buffer == NULL)
    {
        fclose(Fp);

        return NULL;
    }

    size_t Read = fread(Buffer, 1, (size_t)Size, Fp);
    Buffer[Read] = '\0';
    fclose(Fp);

    return Buffer;
}

static bool PyxWriteBinary(
    const char *Path,
    const uint32_t *Instructions,
    size_t Count
){
    FILE *Fp = fopen(Path, "wb");

    if (Fp == NULL)
    {
        printf(":: error: unable to create output file '%s'\n", Path);

        return false;
    }

    size_t Written = fwrite(Instructions, sizeof(uint32_t), Count, Fp);
    fclose(Fp);

    return Written == Count;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("PYriX64 assembler (pyx %s)\n", PYXVER);
        printf("usage: %s <input.asm> [-o output.bin]\n\n", argv[0]);

        if (!PyxRunTests())
        {
            return 1;
        }

        return 0;
    }

    const char *InputPath = NULL;
    const char *OutputPath = "out.bin";

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            OutputPath = argv[++i];
        }
        else if (InputPath == NULL)
        {
            InputPath = argv[i];
        }
    }

    if (InputPath == NULL)
    {
        printf(":: error: no input file specified\n");

        return 1;
    }

    char *Source = PyxReadFile(InputPath);

    if (Source == NULL)
    {
        return 1;
    }

    uint32_t Instructions[4096];
    size_t Count = 0;

    if (!PyxAssemble(Source, Instructions, 4096, &Count))
    {
        free(Source);

        return 1;
    }

    free(Source);

    if (!PyxWriteBinary(OutputPath, Instructions, Count))
    {
        return 1;
    }

    printf(":: assembled %zu instructions to '%s'\n", Count, OutputPath);

    return 0;
}
