/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: main.c
 * DESC.: pyx emulator
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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cfg.h"
#include "cpu.h"
#include "display.h"
#include "enc.h"
#include "pyrix/isa.h"

static bool Pyrix64TestJmp(void)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    const uint32_t Program[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 10),
        Pyrix64EncodeJ(Pyrix64opJmp, 3),
        Pyrix64EncodeI(Pyrix64opMovI, 1, 99),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    if (!Pyrix64Execute(&Cpu, &Ram, Program, ProgramSize))
    {
        return false;
    }

    return Cpu.Registers[1] == 10;
}

static bool Pyrix64TestBeq(void)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    // test taken
    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    const uint32_t ProgramTaken[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 5),
        Pyrix64EncodeI(Pyrix64opMovI, 2, 5),
        Pyrix64EncodeB(Pyrix64opBeq, 1, 2, 4),
        Pyrix64EncodeI(Pyrix64opMovI, 3, 100),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    if (!Pyrix64Execute(&Cpu, &Ram, ProgramTaken, sizeof(ProgramTaken) / sizeof(ProgramTaken[0])))
    {
        return false;
    }

    if (Cpu.Registers[3] != 0)
    {
        return false;
    }

    // test not taken
    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    const uint32_t ProgramNotTaken[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 5),
        Pyrix64EncodeI(Pyrix64opMovI, 2, 6),
        Pyrix64EncodeB(Pyrix64opBeq, 1, 2, 4),
        Pyrix64EncodeI(Pyrix64opMovI, 3, 1),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    if (!Pyrix64Execute(&Cpu, &Ram, ProgramNotTaken, sizeof(ProgramNotTaken) / sizeof(ProgramNotTaken[0])))
    {
        return false;
    }

    return Cpu.Registers[3] == 1;
}

static bool Pyrix64TestBne(void)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    // test taken
    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    const uint32_t ProgramTaken[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 5),
        Pyrix64EncodeI(Pyrix64opMovI, 2, 6),
        Pyrix64EncodeB(Pyrix64opBne, 1, 2, 4),
        Pyrix64EncodeI(Pyrix64opMovI, 3, 100),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    if (!Pyrix64Execute(&Cpu, &Ram, ProgramTaken, sizeof(ProgramTaken) / sizeof(ProgramTaken[0])))
    {
        return false;
    }

    if (Cpu.Registers[3] != 0)
    {
        return false;
    }

    // test not taken
    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    const uint32_t ProgramNotTaken[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 5),
        Pyrix64EncodeI(Pyrix64opMovI, 2, 5),
        Pyrix64EncodeB(Pyrix64opBne, 1, 2, 4),
        Pyrix64EncodeI(Pyrix64opMovI, 3, 1),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    if (!Pyrix64Execute(&Cpu, &Ram, ProgramNotTaken, sizeof(ProgramNotTaken) / sizeof(ProgramNotTaken[0])))
    {
        return false;
    }

    return Cpu.Registers[3] == 1;
}

static bool Pyrix64TestLoop(void)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    // sum 5 down to 1: 5 + 4 + 3 + 2 + 1 = 15
    const uint32_t Program[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 5),
        Pyrix64EncodeI(Pyrix64opMovI, 2, 0),
        Pyrix64EncodeI(Pyrix64opMovI, 3, 1),
        Pyrix64EncodeR(Pyrix64opAdd, 2, 2, 1),
        Pyrix64EncodeR(Pyrix64opSub, 1, 1, 3),
        Pyrix64EncodeB(Pyrix64opBne, 1, 0, 3),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    if (!Pyrix64Execute(&Cpu, &Ram, Program, ProgramSize))
    {
        return false;
    }

    return Cpu.Registers[2] == 15 && Cpu.Registers[1] == 0;
}

static bool Pyrix64RunBranchTests(void)
{
    printf("::  running branch tests\n");

    if (!Pyrix64TestJmp())
    {
        printf(" - JMP test: FAILED\n");
        return false;
    }
    printf(" - JMP test: OK\n");

    if (!Pyrix64TestBeq())
    {
        printf(" - BEQ test: FAILED\n");
        return false;
    }
    printf(" - BEQ test: OK\n");

    if (!Pyrix64TestBne())
    {
        printf(" - BNE test: FAILED\n");
        return false;
    }
    printf(" - BNE test: OK\n");

    if (!Pyrix64TestLoop())
    {
        printf(" - Loop test: FAILED\n");
        return false;
    }
    printf(" - Loop test: OK\n");

    printf("==> All branch tests passed\n\n");

    return true;
}

static bool Pyrix64TestCallRet(void)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    const uint32_t Program[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 10),
        Pyrix64EncodeJ(Pyrix64opCall, 4),
        Pyrix64EncodeR(Pyrix64opAdd, 1, 1, 2),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0),
        // func at 4
        Pyrix64EncodeI(Pyrix64opMovI, 2, 25),
        Pyrix64EncodeI(Pyrix64opRet, 0, 0)
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    if (!Pyrix64Execute(&Cpu, &Ram, Program, ProgramSize))
    {
        return false;
    }

    return Cpu.Registers[1] == 35 && Cpu.Registers[2] == 25;
}

static bool Pyrix64TestPushPop(void)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    const uint32_t Program[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 100),
        Pyrix64EncodeI(Pyrix64opMovI, 2, 200),
        Pyrix64EncodeI(Pyrix64opPush, 1, 0),
        Pyrix64EncodeI(Pyrix64opPush, 2, 0),
        Pyrix64EncodeI(Pyrix64opPop, 3, 0),
        Pyrix64EncodeI(Pyrix64opPop, 4, 0),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    if (!Pyrix64Execute(&Cpu, &Ram, Program, ProgramSize))
    {
        return false;
    }

    return Cpu.Registers[3] == 200 && Cpu.Registers[4] == 100 && Cpu.Sp == Pyrix64RamSize;
}

static bool Pyrix64TestNestedCall(void)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    const uint32_t Program[] =
    {
        Pyrix64EncodeJ(Pyrix64opCall, 3),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0),
        Pyrix64EncodeI(Pyrix64opNop, 0, 0),
        // funcA at 3: saves ra, calls funcB, restores ra, returns
        Pyrix64EncodeI(Pyrix64opPush, Pyrix64RegisterRa, 0),
        Pyrix64EncodeJ(Pyrix64opCall, 9),
        Pyrix64EncodeI(Pyrix64opPop, Pyrix64RegisterRa, 0),
        Pyrix64EncodeI(Pyrix64opMovI, 2, 1),
        Pyrix64EncodeR(Pyrix64opAdd, 1, 1, 2),
        Pyrix64EncodeI(Pyrix64opRet, 0, 0),
        // funcB at 9: sets R1 = 40, returns to funcA
        Pyrix64EncodeI(Pyrix64opMovI, 1, 40),
        Pyrix64EncodeI(Pyrix64opRet, 0, 0)
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    if (!Pyrix64Execute(&Cpu, &Ram, Program, ProgramSize))
    {
        return false;
    }

    return Cpu.Registers[1] == 41 && Cpu.Sp == Pyrix64RamSize;
}

static bool Pyrix64TestStackUnderflow(void)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    const uint32_t Program[] =
    {
        Pyrix64EncodeI(Pyrix64opPop, 1, 0),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    return !Pyrix64Execute(&Cpu, &Ram, Program, ProgramSize);
}

static bool Pyrix64RunStackTests(void)
{
    printf("::  running stack & call tests\n");

    if (!Pyrix64TestCallRet())
    {
        printf(" - CALL / RET test: FAILED\n");
        return false;
    }
    printf(" - CALL / RET test: OK\n");

    if (!Pyrix64TestPushPop())
    {
        printf(" - PUSH / POP test: FAILED\n");
        return false;
    }
    printf(" - PUSH / POP test: OK\n");

    if (!Pyrix64TestNestedCall())
    {
        printf(" - Nested CALL test: FAILED\n");
        return false;
    }
    printf(" - Nested CALL test: OK\n");

    if (!Pyrix64TestStackUnderflow())
    {
        printf(" - Stack underflow test: FAILED\n");
        return false;
    }
    printf(" - Stack underflow test: OK\n");

    printf("==> All stack & call tests passed\n\n");

    return true;
}

static bool Pyrix64LoadBinary(
    const char *Path,
    uint32_t *Program,
    size_t MaxInstructions,
    uint32_t *ProgramSize
){
    FILE *Fp = fopen(Path, "rb");

    if (Fp == NULL)
    {
        printf(":: error: unable to open binary file '%s'\n", Path);

        return false;
    }

    fseek(Fp, 0, SEEK_END);
    long FileSize = ftell(Fp);
    fseek(Fp, 0, SEEK_SET);

    if (FileSize <= 0 || (FileSize % sizeof(uint32_t)) != 0)
    {
        printf(":: error: invalid binary file size\n");
        fclose(Fp);

        return false;
    }

    size_t Count = (size_t)FileSize / sizeof(uint32_t);

    if (Count > MaxInstructions)
    {
        printf(":: error: binary exceeds max instructions\n");
        fclose(Fp);

        return false;
    }

    size_t Read = fread(Program, sizeof(uint32_t), Count, Fp);
    fclose(Fp);

    if (Read != Count)
    {
        printf(":: error: failed to read binary file\n");

        return false;
    }

    *ProgramSize = (uint32_t)Count;

    return true;
}

int main(int argc, char *argv[])
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    if (argc > 1)
    {
        uint32_t Program[16384];
        uint32_t ProgramSize = 0;

        printf("::  pyrix64 pysmu emulator\n");
        printf("::  loading '%s'\n", argv[1]);

        if (!Pyrix64LoadBinary(argv[1], Program, 16384, &ProgramSize))
        {
            return 1;
        }

        size_t CopyBytes = ProgramSize * sizeof(uint32_t);
        if (CopyBytes > Pyrix64RamSize)
        {
            CopyBytes = Pyrix64RamSize;
        }
        memcpy(Ram.Bytes, Program, CopyBytes);

        Pyrix64DisplayInit("PYRIX64 VM Display");

        if (access("bayle.mp4", F_OK) == 0)
        {
            Pyrix64VideoInit("bayle.mp4", 160, 100, 24);
        }

        if (!Pyrix64Execute(&Cpu, &Ram, Program, ProgramSize))
        {
            Pyrix64VideoClose();
            Pyrix64DisplayClose();

            return 1;
        }

        printf("==> Program finished\n\n");

        for (uint8_t i = 1; i < Pyrix64RegisterCount; i++)
        {
            if (Cpu.Registers[i] != 0)
            {
                printf(" - R%u = %" PRIu64 " (0x%" PRIX64 ")\n", i, Cpu.Registers[i], Cpu.Registers[i]);
            }
        }

        Pyrix64DisplayFlush();
        Pyrix64DisplayWaitClose();
        Pyrix64VideoClose();
        Pyrix64DisplayClose();

        return 0;
    }

    /* custom program till we have a compiler */
    const uint32_t Program[] =
    {
        Pyrix64EncodeI(Pyrix64opMovI, 1, 20),
        Pyrix64EncodeI(Pyrix64opMovI, 2, 6),
        Pyrix64EncodeR(Pyrix64opAdd, 3, 1, 2),
        Pyrix64EncodeR(Pyrix64opSub, 4, 1, 2),
        Pyrix64EncodeR(Pyrix64opAnd, 5, 1, 2),
        Pyrix64EncodeR(Pyrix64opOr, 6, 1, 2),
        Pyrix64EncodeR(Pyrix64opXor, 7, 1, 2),
        Pyrix64EncodeR(Pyrix64opNot, 8, 2, 0),
        Pyrix64EncodeI(Pyrix64opStore, 1, 0x100),
        Pyrix64EncodeI(Pyrix64opLoad, 9, 0x100),
        Pyrix64EncodeI(Pyrix64opHalt, 0, 0)
    };

    uint32_t ProgramSize = sizeof(Program) / sizeof(Program[0]);

    printf("::  pyrix64 pysmu emulator\n");
    printf("::  starting program \n");

    if (!Pyrix64Execute(&Cpu, &Ram, Program, ProgramSize))
    {
        return 1;
    }

    printf("==> Program finished\n\n");

    printf(" - R1 = %" PRIu64 "\n", Cpu.Registers[1]);
    printf(" - R2 = %" PRIu64 "\n", Cpu.Registers[2]);
    printf(" - R3 = %" PRIu64 "\n", Cpu.Registers[3]);
    printf(" - R4 = %" PRIu64 "\n", Cpu.Registers[4]);
    printf(" - R5 = %" PRIu64 "\n", Cpu.Registers[5]);
    printf(" - R6 = %" PRIu64 "\n", Cpu.Registers[6]);
    printf(" - R7 = %" PRIu64 "\n", Cpu.Registers[7]);

    printf(" - R8 = 0x%016" PRIX64 "\n", Cpu.Registers[8]);
    printf(" - R9 = %" PRIu64 "\n\n", Cpu.Registers[9]);

    if (!Pyrix64RunBranchTests())
    {
        return 1;
    }

    if (!Pyrix64RunStackTests())
    {
        return 1;
    }

    return 0;
}
