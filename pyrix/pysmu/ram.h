/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: ram.h
 * DESC.: simulated RAM
 *
 * CREATED BY: emex
 * UPDATED BY: emex
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

#ifndef Pyrix64RamH
#define Pyrix64RamH 1

#include <stdbool.h>
#include <stdint.h>

#include "cfg.h"

typedef struct
{
    uint8_t Bytes[Pyrix64RamSize];
} Pyrix64Ram;

void Pyrix64RamInit(Pyrix64Ram *Ram);

// single byte access
bool Pyrix64RamLoadByte(Pyrix64Ram *Ram, uint64_t Address, uint8_t *Value);
bool Pyrix64RamStoreByte(Pyrix64Ram *Ram, uint64_t Address, uint8_t Value);

// 64 bit access, little endian
bool Pyrix64RamLoad(Pyrix64Ram *Ram, uint64_t Address, uint64_t *Value);
bool Pyrix64RamStore(Pyrix64Ram *Ram, uint64_t Address, uint64_t Value);

#endif /*Pyrix64RamH */
