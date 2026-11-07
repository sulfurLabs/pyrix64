/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64
 * FILE: enc.h
 * DESC.: encoding macros
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

#ifndef Pyrix64EncH
#define Pyrix64EncH 1

#include <stdint.h>

// opcode + destination register + register + register
#define Pyrix64EncodeR(Op, Rd, Rs1, Rs2)  \
    (((uint32_t)(Op) << 24) |             \
    ((uint32_t)(Rd)  << 19) |             \
    ((uint32_t)(Rs1) << 14) |             \
    ((uint32_t)(Rs2) << 9))

// opcode + destination register + immediate val
#define Pyrix64EncodeI(Op, Rd, Imm)       \
    (((uint32_t)(Op) << 24) |             \
    ((uint32_t)(Rd)  << 19) |             \
    ((uint32_t)(Imm) & 0x7FFFFu))

// opcode + address
#define Pyrix64EncodeJ(Op, Addr)          \
    (((uint32_t)(Op)   << 24) |           \
    ((uint32_t)(Addr) & 0x3FFFu))

// opcode + register + register + address
#define Pyrix64EncodeB(Op, Rs1, Rs2, Addr) \
    (((uint32_t)(Op)   << 24) |            \
    ((uint32_t)(Rs1)  << 19) |            \
    ((uint32_t)(Rs2)  << 14) |            \
    ((uint32_t)(Addr) & 0x3FFFu))

#endif /*Pyrix64EncH */
