/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pyx
 * FILE: bin.c
 * DESC.: raw binary image output
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
#include <string.h>

#include "internal.h"

bool PyxImageAppendWord(
    PyxImage *Image,
    uint32_t Word
){
    if (Image->Size + 4 > Image->Capacity)
    {
        const size_t NewCapacity = (Image->Capacity == 0) ? 64 : Image->Capacity * 2;
        uint8_t *NewData = realloc(Image->Data, NewCapacity);

        if (NewData == NULL)
        {
            fprintf(stderr, "::  error: out of memory\n");

            return false;
        }

        Image->Data = NewData;
        Image->Capacity = NewCapacity;
    }

    for (size_t i = 0; i < 4; i++)
    {
        Image->Data[Image->Size++] = (uint8_t)((Word >> (8 * i)) & 0xFFu);
    }

    return true;
}

//resets an image to empty
void PyxImageInit(
    PyxImage *Image
){
    Image->Data = NULL;
    Image->Size = 0;
    Image->Capacity = 0;
}

//its raw binary, without any metadata
bool PyxImageWrite(
    const char *Path,
    const PyxImage *Image
){
    FILE *File = fopen(Path, "wb");

    if (File == NULL)
    {
        fprintf(stderr, "::  error: cannot open '%s' for writing\n", Path);

        return false;
    }

    if (Image->Size > 0 && fwrite(Image->Data, 1, Image->Size, File) != Image->Size)
    {
        fprintf(stderr, "::  error: cannot write '%s'\n", Path);

        fclose(File);
        remove(Path);

        return false;
    }

    if (fclose(File) != 0)
    {
        fprintf(stderr, "::  error: cannot write '%s'\n", Path);
        remove(Path);

        return false;
    }

    return true;
}

//releases an image
void PyxImageFree(
    PyxImage *Image
){
    free(Image->Data);
    Image->Data = NULL;
    Image->Size = 0;
    Image->Capacity = 0;
}
