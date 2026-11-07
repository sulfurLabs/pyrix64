/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: ram.c
 * DESC.: simulated RAM
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
#include <string.h>

#include "ram.h"

void Pyrix64RamInit(Pyrix64Ram *Ram)
{
    memset(Ram->Bytes, 0, sizeof(Ram->Bytes));
}

// little endian
bool Pyrix64RamLoad(Pyrix64Ram *Ram, uint16_t Address, uint64_t *Value)
{
    uint64_t Result = 0;

    if (Address + Pyrix64RamAccessSize > Pyrix64RamSize)
    {
        printf(":: error: RAM load out of bounds (0x%04X)\n", Address);

        return false;
    }
    for (uint32_t i = 0; i < Pyrix64RamAccessSize; i++)
    {
        Result |= (uint64_t)Ram->Bytes[Address + i] << (8 * i);
    }

    *Value = Result;

    return true;
}

bool Pyrix64RamStore(Pyrix64Ram *Ram, uint16_t Address, uint64_t Value)
{
    if (Address + Pyrix64RamAccessSize > Pyrix64RamSize)
    {
        printf(":: error: RAM store out of bounds (0x%04X)\n", Address);

        return false;
    }
    for (uint32_t i = 0; i < Pyrix64RamAccessSize; i++)
    {
        Ram->Bytes[Address + i] = (uint8_t)((Value >> (8 * i)) & 0xFFu);
    }

    return true;
}
