/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: video.c
 * DESC.: video stream decoder and frame buffer
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "video.h"
#include "vga_font.h"

static uint8_t *VideoBuffer = NULL;
static size_t VideoBufferSize = 0;
static size_t VideoBufferPos = 0;

static uint8_t QuantizeRgbToVga(uint8_t R, uint8_t G, uint8_t B)
{
    uint32_t BestDist = UINT32_MAX;
    uint8_t BestIndex = 0;

    for (int i = 0; i < 16; i++)
    {
        uint32_t C = Pyrix64VgaPalette[i];
        int Cr = (int)((C >> 16) & 0xFF);
        int Cg = (int)((C >> 8) & 0xFF);
        int Cb = (int)(C & 0xFF);

        int Dr = (int)R - Cr;
        int Dg = (int)G - Cg;
        int Db = (int)B - Cb;
        uint32_t Dist = (uint32_t)(Dr * Dr + Dg * Dg + Db * Db);

        if (Dist < BestDist)
        {
            BestDist = Dist;
            BestIndex = (uint8_t)i;
        }
    }

    return BestIndex;
}

bool Pyrix64VideoInit(const char *Filename, uint32_t Width, uint32_t Height, uint32_t Fps)
{
    if (VideoBuffer != NULL)
    {
        free(VideoBuffer);
        VideoBuffer = NULL;
        VideoBufferSize = 0;
        VideoBufferPos = 0;
    }

    char Cmd[1024];
    snprintf(Cmd, sizeof(Cmd),
        "ffmpeg -v error -i \"%s\" -vf \"scale=%u:%u:force_original_aspect_ratio=decrease,pad=%u:%u:(ow-iw)/2:(oh-ih)/2,format=rgb24\" -f rawvideo -r %u -",
        Filename, Width, Height, Width, Height, Fps);

    FILE *Pipe = popen(Cmd, "r");
    if (Pipe == NULL)
    {
        printf(":: error: unable to launch ffmpeg for video decoding\n");
        return false;
    }

    size_t Capacity = Width * Height * 3 * Fps * 12; // ~12 seconds
    VideoBuffer = malloc(Capacity);
    if (VideoBuffer == NULL)
    {
        pclose(Pipe);
        return false;
    }

    size_t TotalRead = 0;
    size_t Chunk = 64 * 1024;
    while (1)
    {
        if (TotalRead + Chunk > Capacity)
        {
            size_t NewCap = Capacity * 2;
            uint8_t *NewBuf = realloc(VideoBuffer, NewCap);
            if (NewBuf == NULL)
            {
                break;
            }
            VideoBuffer = NewBuf;
            Capacity = NewCap;
        }

        size_t n = fread(VideoBuffer + TotalRead, 1, Chunk, Pipe);
        if (n == 0)
        {
            break;
        }
        TotalRead += n;
    }

    pclose(Pipe);

    size_t FrameBytes = Width * Height * 3;
    if (FrameBytes > 0)
    {
        VideoBufferSize = (TotalRead / FrameBytes) * FrameBytes;
    }
    else
    {
        VideoBufferSize = TotalRead;
    }

    VideoBufferPos = 0;

    size_t Frames = (FrameBytes > 0) ? VideoBufferSize / FrameBytes : 0;
    printf("::  video '%s' decoded (%zu frames at %ux%u)\n", Filename, Frames, Width, Height);

    return (Frames > 0);
}

uint32_t Pyrix64VideoReadPixelRGB(void)
{
    if (VideoBuffer == NULL || VideoBufferSize < 3)
    {
        return 0;
    }

    if (VideoBufferPos + 3 > VideoBufferSize)
    {
        VideoBufferPos = 0; // Rewind to loop video
    }

    uint8_t R = VideoBuffer[VideoBufferPos++];
    uint8_t G = VideoBuffer[VideoBufferPos++];
    uint8_t B = VideoBuffer[VideoBufferPos++];

    return ((uint32_t)R << 16) | ((uint32_t)G << 8) | (uint32_t)B;
}

uint8_t Pyrix64VideoReadPixelVGA(void)
{
    if (VideoBuffer == NULL || VideoBufferSize < 3)
    {
        return 0;
    }

    if (VideoBufferPos + 3 > VideoBufferSize)
    {
        VideoBufferPos = 0; // Rewind to loop video
    }

    uint8_t R = VideoBuffer[VideoBufferPos++];
    uint8_t G = VideoBuffer[VideoBufferPos++];
    uint8_t B = VideoBuffer[VideoBufferPos++];

    return QuantizeRgbToVga(R, G, B);
}

void Pyrix64VideoRewind(void)
{
    VideoBufferPos = 0;
}

void Pyrix64VideoClose(void)
{
    if (VideoBuffer != NULL)
    {
        free(VideoBuffer);
        VideoBuffer = NULL;
        VideoBufferSize = 0;
        VideoBufferPos = 0;
    }
}
