/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: display.h
 * DESC.: Wayland and X11 native display interface
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

#ifndef Pyrix64DisplayH
#define Pyrix64DisplayH 1

#include <stdbool.h>
#include <stdint.h>

#include "vga.h"
#include "video.h"

#define Pyrix64DisplayWidth 640
#define Pyrix64DisplayHeight 400

#define Pyrix64ScreenAddress       0x0E00
#define Pyrix64ScreenColorAddress  0x0E01
#define Pyrix64PixelXAddress       0x0E02
#define Pyrix64PixelYAddress       0x0E03
#define Pyrix64PixelColorAddress   0x0E04
#define Pyrix64FrameSyncAddress    0x0E05
#define Pyrix64PixelScaleAddress   0x0E06
#define Pyrix64VideoStreamAddress  0x0E07

bool Pyrix64DisplayInit(const char *Title);
uint32_t *Pyrix64DisplayGetFrameBuffer(void);
void Pyrix64DisplayFrameSync(uint32_t SleepMs);
void Pyrix64DisplayFlush(void);
bool Pyrix64DisplayPoll(void);
void Pyrix64DisplayWaitClose(void);
void Pyrix64DisplayClose(void);

// Compatibility wrappers delegating to vga module
#define Pyrix64DisplayPutChar(c)         Pyrix64VgaPutChar(c)
#define Pyrix64DisplaySetColor(fg, bg)   Pyrix64VgaSetColor((fg), (bg))
#define Pyrix64DisplayClear(bg)          Pyrix64VgaClear(bg)
#define Pyrix64DisplaySetPixelX(x)       Pyrix64VgaSetPixelX(x)
#define Pyrix64DisplaySetPixelY(y)       Pyrix64VgaSetPixelY(y)
#define Pyrix64DisplaySetPixelScale(s)   Pyrix64VgaSetPixelScale(s)
#define Pyrix64DisplayPlotColorRGB(rgb)  Pyrix64VgaPlotRGB(rgb)
#define Pyrix64DisplayPlotColorVGA(idx)  Pyrix64VgaPlotPalette(idx)
#define Pyrix64DisplayPlotColor(c)       Pyrix64VgaPlotColor(c)

#endif /* Pyrix64DisplayH */
