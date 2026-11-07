/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: video.h
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

#ifndef Pyrix64VideoH
#define Pyrix64VideoH 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool Pyrix64VideoInit(const char *Filename, uint32_t Width, uint32_t Height, uint32_t Fps);
uint32_t Pyrix64VideoReadPixelRGB(void);
uint8_t Pyrix64VideoReadPixelVGA(void);
void Pyrix64VideoRewind(void);
void Pyrix64VideoClose(void);

#endif /* Pyrix64VideoH */
