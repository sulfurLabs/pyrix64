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

//privilege levels (PL)
typedef enum
{
    Pyrix64PLUser = 0,
    Pyrix64PLDriver = 1,
    Pyrix64PLKernel = 2,
    Pyrix64PLSecureMonitor = 3
} Pyrix64PrivilegeLevel;

typedef struct
{
    uint64_t Registers[Pyrix64RegisterCount]; //64 bit, r0 to r31
    uint64_t StackPointers[Pyrix64StackPointerCount]; //SP0-3 each per pl
    uint64_t Fp; // frame pointer
    uint64_t Pc; // pc == program counter, not personal computer
    uint64_t Status; // status register, bit layout to be defined later
    uint64_t Epc; // exception program counter
    uint64_t Cause; // reason of an interrupt/exception
    uint64_t Tvec; // trap vector
    Pyrix64PrivilegeLevel CurrentPL; // current privilege level
    bool Halted; // true if it really stopped

} Pyrix64Cpu;

void Pyrix64CpuInit(Pyrix64Cpu *Cpu);

uint64_t Pyrix64RegRead(
    const Pyrix64Cpu *Cpu,
    uint8_t RegIndex
);

void Pyrix64RegWrite(
    Pyrix64Cpu *Cpu,
    uint8_t RegIndex,
    uint64_t Value
);

bool Pyrix64StackPointerRead(
    const Pyrix64Cpu *Cpu,
    uint8_t SpIndex,
    uint64_t *Value
);

bool Pyrix64StackPointerWrite(
    Pyrix64Cpu *Cpu,
    uint8_t SpIndex,
    uint64_t Value
);

uint8_t Pyrix64ActiveStackPointerIndex(
    const Pyrix64Cpu *Cpu
);

uint64_t Pyrix64ActiveStackPointerRead(
    const Pyrix64Cpu *Cpu
);

void Pyrix64ActiveStackPointerWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
);

Pyrix64PrivilegeLevel Pyrix64PrivilegeLevelRead(
    const Pyrix64Cpu *Cpu
);

bool Pyrix64PrivilegeLevelWrite(
    Pyrix64Cpu *Cpu,
    Pyrix64PrivilegeLevel PL
);

uint64_t Pyrix64StatusRead(
    const Pyrix64Cpu *Cpu
);
void Pyrix64StatusWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
);

uint64_t Pyrix64EpcRead(
    const Pyrix64Cpu *Cpu
);
void Pyrix64EpcWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
);

uint64_t Pyrix64CauseRead(
    const Pyrix64Cpu *Cpu
);
void Pyrix64CauseWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
);

uint64_t Pyrix64TvecRead(
    const Pyrix64Cpu *Cpu
);
void Pyrix64TvecWrite(
    Pyrix64Cpu *Cpu,
    uint64_t Value
);

bool Pyrix64Execute(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram,
    const uint32_t *Program,
    uint32_t ProgramSize
);

#endif /*Pyrix64CpuH */
