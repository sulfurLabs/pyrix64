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
    Pyrix64opNop     = 0x00, // nothing
    Pyrix64opHalt    = 0x01, // stops
    Pyrix64opMov     = 0x10, // copy one register to another
    Pyrix64opAdd     = 0x11, // addition
    Pyrix64opSub     = 0x12, // subtraction
    Pyrix64opAnd     = 0x13, // keep 2 bits if they r 1
    Pyrix64opOr      = 0x14, // bits which r 1 in either
    Pyrix64opXor     = 0x15, // bits which are different
    Pyrix64opNot     = 0x16, // flips
    Pyrix64opMovI    = 0x20, // number into register
    Pyrix64opLoad    = 0x30, // from memory
    Pyrix64opStore   = 0x31, // into memory
    Pyrix64opJmp     = 0x40, // jump
    Pyrix64opBeq     = 0x41, // jump if the values match
    Pyrix64opBne     = 0x42, // jump if the values dont match
    Pyrix64opPush    = 0x50, // push a register
    Pyrix64opPop     = 0x51, // pop a register
    Pyrix64opPushFp  = 0x52, // push the frame pointer
    Pyrix64opPopFp   = 0x53, // pop the frame pointer


    // 64bit instructions
    Pyrix64opPushM   = 0xE0, // push multiple gprs
    Pyrix64opPopM    = 0xE1  // pop multiple gprs

} Pyrix64Opcode;

#endif /*Pyrix64IsaH */
