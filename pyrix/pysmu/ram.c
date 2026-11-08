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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "ram.h"

void Pyrix64RamInit(Pyrix64Ram *Ram)
{
    memset(Ram->Bytes, 0, sizeof(Ram->Bytes));
}

//AccessSize from Adress needs to be checked that it stays in ram
// shouldnt overflow
bool Pyrix64RamRangeOk(
    uint64_t Address,
    uint64_t AccessSize
){
    if (Address >= Pyrix64RamSize)
    {
        return false;
    }
    if (AccessSize > (uint64_t)Pyrix64RamSize - Address)
    {
        return false;
    }

    return true;
}

//reads one byte
// false if out of bounds
bool Pyrix64RamLoadByte(
    Pyrix64Ram *Ram,
    uint64_t Address,
    uint8_t *Value
){
    if (!Pyrix64RamRangeOk(Address, 1))
    {
        printf(":: error: RAM load out of bounds (0x%" PRIX64 ")\n", Address);

        return false;
    }

    *Value = Ram->Bytes[Address];

    return true;
}
//writes one byte
// false if out of bounds
bool Pyrix64RamStoreByte(
    Pyrix64Ram *Ram,
    uint64_t Address,
    uint8_t Value
){
    if (!Pyrix64RamRangeOk(Address, 1))
    {
        printf(":: error: RAM store out of bounds (0x%" PRIX64 ")\n", Address);

        return false;
    }

    Ram->Bytes[Address] = Value;

    return true;
}

//little endian
bool Pyrix64RamLoad(
    Pyrix64Ram *Ram,
    uint64_t Address,
    uint64_t *Value
){
    uint64_t Result = 0;

    if (!Pyrix64RamRangeOk(Address, Pyrix64RamAccessSize))
    {
        printf(":: error: RAM load out of bounds (0x%" PRIX64 ")\n", Address);

        return false;
    }
    // range is checked, so this cant run past the end
    for (uint64_t i = 0; i < Pyrix64RamAccessSize; i++)
    {
        Result |= (uint64_t)Ram->Bytes[Address + i] << (8 * i);
    }

    *Value = Result;

    return true;
}

//32 bit access
bool Pyrix64RamLoad32(
    Pyrix64Ram *Ram,
    uint64_t Address,
    uint32_t *Value
){
    uint32_t Result = 0;

    if (!Pyrix64RamRangeOk(Address, 4))
    {
        printf(":: error: RAM load out of bounds (0x%" PRIX64 ")\n", Address);

        return false;
    }
    for (uint64_t i = 0; i < 4; i++)
    {
        Result |= (uint32_t)Ram->Bytes[Address + i] << (8 * i);
    }

    *Value = Result;

    return true;
}

//32 bit access
bool Pyrix64RamStore32(
    Pyrix64Ram *Ram,
    uint64_t Address,
    uint32_t Value
){
    if (!Pyrix64RamRangeOk(Address, 4))
    {
        printf(":: error: RAM store out of bounds (0x%" PRIX64 ")\n", Address);

        return false;
    }
    for (uint64_t i = 0; i < 4; i++)
    {
        Ram->Bytes[Address + i] = (uint8_t)((Value >> (8 * i)) & 0xFFu);
    }

    return true;
}

//less significant byte first
bool Pyrix64RamStore(
    Pyrix64Ram *Ram,
    uint64_t Address,
    uint64_t Value
){
    if (!Pyrix64RamRangeOk(Address, Pyrix64RamAccessSize))
    {
        printf(":: error: RAM store out of bounds (0x%" PRIX64 ")\n", Address);

        return false;
    }
    for (uint64_t i = 0; i < Pyrix64RamAccessSize; i++)
    {
        Ram->Bytes[Address + i] = (uint8_t)((Value >> (8 * i)) & 0xFFu);
    }

    return true;
}
