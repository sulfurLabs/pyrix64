/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64
 * FILE: alu.v
 * DESC.: ALU
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

#ifndef Pyrix64IsaH
#define Pyrix64IsaH 1

#include <stdint.h>

typedef enum
{
    Pyrix64opNop   = 0x00,
    Pyrix64opHalt  = 0x01,
    Pyrix64opMov   = 0x10,
    Pyrix64opAdd   = 0x11,
    Pyrix64opSub   = 0x12,
    Pyrix64opAnd   = 0x13,
    Pyrix64opOr    = 0x14,
    Pyrix64opXor   = 0x15,
    Pyrix64opNot   = 0x16,
    Pyrix64opMovI  = 0x20,
    Pyrix64opLoad  = 0x30,
    Pyrix64opStore = 0x31,
    Pyrix64opJmp   = 0x40,
    Pyrix64opBeq   = 0x41,
    Pyrix64opBne   = 0x42

} Pyrix64Opcode;

#endif /*Pyrix64IsaH */
