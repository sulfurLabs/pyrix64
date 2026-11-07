/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: vga.h
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

#ifndef Pyrix64VgaH
#define Pyrix64VgaH 1

#include <stdbool.h>
#include <stdint.h>

#include "vga_font.h"

#define Pyrix64DisplayCols 80
#define Pyrix64DisplayRows 25
#define Pyrix64FontWidth 8
#define Pyrix64FontHeight 16

void Pyrix64VgaInit(void);
void Pyrix64VgaSetColor(uint8_t FgColor, uint8_t BgColor);
void Pyrix64VgaClear(uint8_t BgColor);
void Pyrix64VgaPutChar(char Char);
void Pyrix64VgaDrawChar(int Col, int Row, uint8_t C, uint32_t FgColor, uint32_t BgColor);
void Pyrix64VgaScrollUp(void);

void Pyrix64VgaSetPixelX(uint32_t X);
void Pyrix64VgaSetPixelY(uint32_t Y);
void Pyrix64VgaSetPixelScale(uint32_t Scale);
void Pyrix64VgaPlotRGB(uint32_t Rgb);
void Pyrix64VgaPlotPalette(uint8_t VgaIndex);
void Pyrix64VgaPlotColor(uint32_t Color);

#endif /* Pyrix64VgaH */
