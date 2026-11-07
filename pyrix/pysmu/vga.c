/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: vga.c
 * DESC.: VGA text console and Mode 13h graphics engine
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

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "vga.h"
#include "display.h"

static int CursorCol = 0;
static int CursorRow = 0;
static uint8_t CurrentFgIndex = Pyrix64ColorLightGray;
static uint8_t CurrentBgIndex = Pyrix64ColorBlack;
static uint32_t CurrentFgColor = 0xAAAAAA;
static uint32_t CurrentBgColor = 0x000000;

static uint32_t PixelX = 0;
static uint32_t PixelY = 0;
static uint32_t PixelScale = 1;

// ANSI escape state
static int AnsiState = 0;
static int AnsiArgs[4] = {0};
static int AnsiArgCount = 0;

void Pyrix64VgaInit(void)
{
    CursorCol = 0;
    CursorRow = 0;
    CurrentFgIndex = Pyrix64ColorLightGray;
    CurrentBgIndex = Pyrix64ColorBlack;
    CurrentFgColor = Pyrix64VgaPalette[CurrentFgIndex];
    CurrentBgColor = Pyrix64VgaPalette[CurrentBgIndex];
    PixelX = 0;
    PixelY = 0;
    PixelScale = 1;
    AnsiState = 0;
    AnsiArgCount = 0;
}

void Pyrix64VgaSetColor(uint8_t FgColor, uint8_t BgColor)
{
    CurrentFgIndex = FgColor & 0x0F;
    CurrentBgIndex = BgColor & 0x0F;
    CurrentFgColor = Pyrix64VgaPalette[CurrentFgIndex];
    CurrentBgColor = Pyrix64VgaPalette[CurrentBgIndex];
}

void Pyrix64VgaClear(uint8_t BgColor)
{
    CurrentBgIndex = BgColor & 0x0F;
    CurrentBgColor = Pyrix64VgaPalette[CurrentBgIndex];

    uint32_t *FrameBuffer = Pyrix64DisplayGetFrameBuffer();
    if (FrameBuffer != NULL)
    {
        for (size_t i = 0; i < Pyrix64DisplayWidth * Pyrix64DisplayHeight; i++)
        {
            FrameBuffer[i] = CurrentBgColor;
        }
    }

    CursorCol = 0;
    CursorRow = 0;
}

void Pyrix64VgaDrawChar(int Col, int Row, uint8_t C, uint32_t FgColor, uint32_t BgColor)
{
    uint32_t *FrameBuffer = Pyrix64DisplayGetFrameBuffer();
    if (FrameBuffer == NULL || Col < 0 || Col >= Pyrix64DisplayCols || Row < 0 || Row >= Pyrix64DisplayRows)
    {
        return;
    }

    const uint8_t *Glyph = Pyrix64VgaFont8x16[C];
    int StartX = Col * Pyrix64FontWidth;
    int StartY = Row * Pyrix64FontHeight;

    for (int y = 0; y < Pyrix64FontHeight; y++)
    {
        uint8_t RowBits = Glyph[y];
        uint32_t *FbRow = &FrameBuffer[(StartY + y) * Pyrix64DisplayWidth + StartX];

        for (int x = 0; x < Pyrix64FontWidth; x++)
        {
            FbRow[x] = (RowBits & (0x80 >> x)) ? FgColor : BgColor;
        }
    }
}

void Pyrix64VgaScrollUp(void)
{
    uint32_t *FrameBuffer = Pyrix64DisplayGetFrameBuffer();
    if (FrameBuffer == NULL)
    {
        return;
    }

    size_t MovePixels = Pyrix64DisplayWidth * (Pyrix64DisplayHeight - Pyrix64FontHeight);
    memmove(FrameBuffer, FrameBuffer + Pyrix64DisplayWidth * Pyrix64FontHeight, MovePixels * sizeof(uint32_t));

    for (size_t i = MovePixels; i < Pyrix64DisplayWidth * Pyrix64DisplayHeight; i++)
    {
        FrameBuffer[i] = CurrentBgColor;
    }
}

void Pyrix64VgaSetPixelX(uint32_t X)
{
    PixelX = X;
}

void Pyrix64VgaSetPixelY(uint32_t Y)
{
    PixelY = Y;
}

void Pyrix64VgaSetPixelScale(uint32_t Scale)
{
    PixelScale = (Scale > 0) ? Scale : 1;
}

static void PlotBlock(uint32_t Rgb)
{
    uint32_t *FrameBuffer = Pyrix64DisplayGetFrameBuffer();
    if (FrameBuffer == NULL)
    {
        return;
    }

    uint32_t Scale = PixelScale;
    uint32_t BaseX = PixelX * Scale;
    uint32_t BaseY = PixelY * Scale;

    if (BaseX < Pyrix64DisplayWidth && BaseY < Pyrix64DisplayHeight)
    {
        for (uint32_t dy = 0; dy < Scale && (BaseY + dy) < Pyrix64DisplayHeight; dy++)
        {
            uint32_t *Row = &FrameBuffer[(BaseY + dy) * Pyrix64DisplayWidth + BaseX];
            for (uint32_t dx = 0; dx < Scale && (BaseX + dx) < Pyrix64DisplayWidth; dx++)
            {
                Row[dx] = Rgb;
            }
        }
    }

    // Auto-advance
    PixelX++;
    if (PixelX >= Pyrix64DisplayWidth / Scale)
    {
        PixelX = 0;
        PixelY++;
        if (PixelY >= Pyrix64DisplayHeight / Scale)
        {
            PixelY = 0;
        }
    }
}

void Pyrix64VgaPlotRGB(uint32_t Rgb)
{
    PlotBlock(Rgb & 0xFFFFFF);
}

void Pyrix64VgaPlotPalette(uint8_t VgaIndex)
{
    PlotBlock(Pyrix64VgaPalette[VgaIndex & 0x0F]);
}

void Pyrix64VgaPlotColor(uint32_t Color)
{
    Pyrix64VgaPlotRGB(Color);
}

void Pyrix64VgaPutChar(char Char)
{
    uint32_t *FrameBuffer = Pyrix64DisplayGetFrameBuffer();
    if (FrameBuffer == NULL)
    {
        putchar(Char);
        fflush(stdout);

        return;
    }

    // Process ANSI CSI escape sequence
    if (AnsiState == 0)
    {
        if ((unsigned char)Char == 0x1B)
        {
            AnsiState = 1;

            return;
        }
    }
    else if (AnsiState == 1)
    {
        if (Char == '[')
        {
            AnsiState = 2;
            AnsiArgCount = 0;
            AnsiArgs[0] = 0;

            return;
        }
        else
        {
            AnsiState = 0;
        }
    }
    else if (AnsiState == 2)
    {
        if (isdigit((unsigned char)Char))
        {
            AnsiArgs[AnsiArgCount] = AnsiArgs[AnsiArgCount] * 10 + (Char - '0');

            return;
        }
        else if (Char == ';')
        {
            if (AnsiArgCount < 3)
            {
                AnsiArgCount++;
            }
            AnsiArgs[AnsiArgCount] = 0;

            return;
        }
        else if (Char == 'm')
        {
            for (int k = 0; k <= AnsiArgCount; k++)
            {
                int Code = AnsiArgs[k];

                if (Code == 0)
                {
                    CurrentFgIndex = Pyrix64ColorLightGray;
                    CurrentBgIndex = Pyrix64ColorBlack;
                }
                else if (Code == 1)
                {
                    if (CurrentFgIndex < 8)
                    {
                        CurrentFgIndex += 8;
                    }
                }
                else if (Code >= 30 && Code <= 37)
                {
                    const uint8_t AnsiToVga[8] = {
                        Pyrix64ColorBlack,
                        Pyrix64ColorRed,
                        Pyrix64ColorGreen,
                        Pyrix64ColorBrown,
                        Pyrix64ColorBlue,
                        Pyrix64ColorMagenta,
                        Pyrix64ColorCyan,
                        Pyrix64ColorLightGray
                    };
                    CurrentFgIndex = AnsiToVga[Code - 30];
                }
                else if (Code >= 90 && Code <= 97)
                {
                    const uint8_t AnsiToVgaBright[8] = {
                        Pyrix64ColorDarkGray,
                        Pyrix64ColorBrightRed,
                        Pyrix64ColorBrightGreen,
                        Pyrix64ColorYellow,
                        Pyrix64ColorBrightBlue,
                        Pyrix64ColorBrightMagenta,
                        Pyrix64ColorBrightCyan,
                        Pyrix64ColorBrightWhite
                    };
                    CurrentFgIndex = AnsiToVgaBright[Code - 90];
                }
                else if (Code >= 40 && Code <= 47)
                {
                    const uint8_t AnsiToVga[8] = {
                        Pyrix64ColorBlack,
                        Pyrix64ColorRed,
                        Pyrix64ColorGreen,
                        Pyrix64ColorBrown,
                        Pyrix64ColorBlue,
                        Pyrix64ColorMagenta,
                        Pyrix64ColorCyan,
                        Pyrix64ColorLightGray
                    };
                    CurrentBgIndex = AnsiToVga[Code - 40];
                }
                else if (Code >= 100 && Code <= 107)
                {
                    const uint8_t AnsiToVgaBright[8] = {
                        Pyrix64ColorDarkGray,
                        Pyrix64ColorBrightRed,
                        Pyrix64ColorBrightGreen,
                        Pyrix64ColorYellow,
                        Pyrix64ColorBrightBlue,
                        Pyrix64ColorBrightMagenta,
                        Pyrix64ColorBrightCyan,
                        Pyrix64ColorBrightWhite
                    };
                    CurrentBgIndex = AnsiToVgaBright[Code - 100];
                }
            }

            CurrentFgColor = Pyrix64VgaPalette[CurrentFgIndex & 0x0F];
            CurrentBgColor = Pyrix64VgaPalette[CurrentBgIndex & 0x0F];
            AnsiState = 0;

            return;
        }
        else if (Char == 'J')
        {
            if (AnsiArgs[0] == 2)
            {
                Pyrix64VgaClear(CurrentBgIndex);
            }

            AnsiState = 0;

            return;
        }
        else if (Char == 'H' || Char == 'f')
        {
            int r = (AnsiArgs[0] > 0) ? AnsiArgs[0] - 1 : 0;
            int c = (AnsiArgCount > 0 && AnsiArgs[1] > 0) ? AnsiArgs[1] - 1 : 0;

            if (r < 0) r = 0;
            if (r >= Pyrix64DisplayRows) r = Pyrix64DisplayRows - 1;
            if (c < 0) c = 0;
            if (c >= Pyrix64DisplayCols) c = Pyrix64DisplayCols - 1;

            CursorRow = r;
            CursorCol = c;
            AnsiState = 0;

            return;
        }
        else
        {
            AnsiState = 0;

            return;
        }
    }

    if (Char == '\n')
    {
        CursorCol = 0;
        CursorRow++;

        if (CursorRow >= Pyrix64DisplayRows)
        {
            Pyrix64VgaScrollUp();
            CursorRow = Pyrix64DisplayRows - 1;
        }

        return;
    }

    if (Char == '\r')
    {
        CursorCol = 0;

        return;
    }

    if (Char == '\b')
    {
        if (CursorCol > 0)
        {
            CursorCol--;
            Pyrix64VgaDrawChar(CursorCol, CursorRow, ' ', CurrentFgColor, CurrentBgColor);
        }

        return;
    }

    if (Char == '\t')
    {
        CursorCol = (CursorCol + 8) & ~7;

        if (CursorCol >= Pyrix64DisplayCols)
        {
            CursorCol = 0;
            CursorRow++;

            if (CursorRow >= Pyrix64DisplayRows)
            {
                Pyrix64VgaScrollUp();
                CursorRow = Pyrix64DisplayRows - 1;
            }
        }

        return;
    }

    Pyrix64VgaDrawChar(CursorCol, CursorRow, (uint8_t)Char, CurrentFgColor, CurrentBgColor);
    CursorCol++;

    if (CursorCol >= Pyrix64DisplayCols)
    {
        CursorCol = 0;
        CursorRow++;

        if (CursorRow >= Pyrix64DisplayRows)
        {
            Pyrix64VgaScrollUp();
            CursorRow = Pyrix64DisplayRows - 1;
        }
    }
}
