/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: cpu.h
 * DESC.: CPU state
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

#ifndef Pyrix64CpuH
#define Pyrix64CpuH 1

#include <stdbool.h>
#include <stdint.h>

#include "ram.h"
#include "cfg.h"

typedef struct
{
    uint64_t Registers[Pyrix64RegisterCount]; //64 bit, r0 to r31
    uint64_t Sp; // stack pointer
    uint64_t Fp; // frame pointer
    uint64_t Pc; // pc == program counter, not personal computer
    bool Halted; // true if it really stopped

} Pyrix64Cpu;

uint64_t Pyrix64RegRead(
    const Pyrix64Cpu *Cpu,
    uint8_t RegIndex
);

void Pyrix64RegWrite(
    Pyrix64Cpu *Cpu,
    uint8_t RegIndex,
    uint64_t Value
);

bool Pyrix64Execute(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    const uint32_t *Program,
    uint32_t ProgramSize
);

#endif /*Pyrix64CpuH */
