/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: bin.c
 * DESC.: raw bin executable loader
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
#include <stdlib.h>

#include "cfg.h"
#include "ram.h"

#include "exec.h"

bool Pyrix64ExecLoad(
    const char *Path,
    Pyrix64Ram *Ram
){
    //rn we only have bin
    // but when we have e.g elf, we need to get exec.c
    return Pyrix64ExecLoadBin(Path, Ram);
}

static bool Pyrix64ExecReadFile(
    const char *Path,
    uint8_t **Data,
    size_t *Size
){
    FILE *File = fopen(Path, "rb");

    if (File == NULL)
    {
        fprintf(stderr, "::  error: cannot open '%s'\n", Path);

        return false;
    }

    if (fseek(File, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "::  error: cannot read '%s'\n", Path);
        fclose(File);

        return false;
    }

    const long FileSize = ftell(File);

    if (FileSize < 0)
    {
        fprintf(stderr, "::  error: cannot read '%s'\n", Path);
        fclose(File);

        return false;
    }

    rewind(File);

    if (FileSize > Pyrix64RamSize)
    {
        fprintf(stderr, "::  error: '%s' does not fit into ram (%ld bytes, ram holds %u)\n", Path, FileSize, Pyrix64RamSize);
        fclose(File);

        return false;
    }

    uint8_t *Content = NULL;

    if (FileSize > 0)
    {
        Content = malloc((size_t)FileSize);

        if (Content == NULL)
        {
            fprintf(stderr, "::  error: out of memory\n");
            fclose(File);

            return false;
        }

        if (fread(Content, 1, (size_t)FileSize, File) != (size_t)FileSize)
        {
            fprintf(stderr, "::  error: cannot read '%s'\n", Path);
            free(Content);
            fclose(File);

            return false;
        }
    }

    fclose(File);

    *Data = Content;
    *Size = (size_t)FileSize;

    return true;
}

bool Pyrix64ExecLoadBin(
    const char *Path,
    Pyrix64Ram *Ram
){
    uint8_t *Data = NULL;
    size_t Size = 0;

    if (!Pyrix64ExecReadFile(Path, &Data, &Size))
    {
        return false;
    }

    if (!Pyrix64RamRangeOk(Pyrix64ProgramStart, Size))
    {
        fprintf(stderr, "::  error: '%s' does not fit into ram (%zu bytes, ram holds %u)\n", Path, Size, Pyrix64RamSize);
        free(Data);

        return false;
    }

    for (size_t i = 0; i < Size; i++)
    {
        if (!Pyrix64RamStoreByte(Ram, Pyrix64ProgramStart + i, Data[i]))
        {
            fprintf(stderr, "::  error: cannot load '%s' into ram\n", Path);
            free(Data);

            return false;
        }
    }

    free(Data);

    return true;
}
