/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: main.c
 * DESC.: pyrix64 emulator
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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cfg.h"
#include "cpu.h"
#include "instr.h"
#include "ram.h"
#include "exec/exec.h"

static const uint32_t Pyrix64DemoProgramSize =
    sizeof(Pyrix64DemoProgram) /
    sizeof(Pyrix64DemoProgram[0])
;

static void Pyrix64PrintRegisters(const Pyrix64Cpu *Cpu )
{
    printf("  > R1 = %" PRIu64 "\n",       Cpu->Registers[1]);
    printf("  > R2 = %" PRIu64 "\n",       Cpu->Registers[2]);
    printf("  > R3 = %" PRIu64 "\n",       Cpu->Registers[3]);
    printf("  > R4 = %" PRIu64 "\n",       Cpu->Registers[4]);
    printf("  > R5 = %" PRIu64 "\n",       Cpu->Registers[5]);
    printf("  > R6 = %" PRIu64 "\n",       Cpu->Registers[6]);
    printf("  > R7 = %" PRIu64 "\n",       Cpu->Registers[7]);
    printf("  > R8 = 0x%016" PRIX64 "\n",  Cpu->Registers[8]);
    printf("  > R9 = %" PRIu64 "\n",       Cpu->Registers[9]);
}

static bool Pyrix64RunCpu(
    Pyrix64Cpu *Cpu,
    Pyrix64Ram *Ram
){
    if (!Pyrix64Execute(Cpu, Ram))
    {
        return false;
    }

    printf("==> Program finished\n\n");
    Pyrix64PrintRegisters(Cpu);

    return true;
}

static bool Pyrix64RunFirmware(const char *Path)
{
    Pyrix64Cpu Cpu;
    Pyrix64Ram Ram;

    Pyrix64CpuInit(&Cpu);
    Pyrix64RamInit(&Ram);

    printf("::  pyrix64 pysmu emulator\n");
    printf("::  loading firmware '%s'\n", Path);

    if (!Pyrix64ExecLoad(Path, &Ram))
    {
        return false;
    }

    printf("::  starting program... \n");

    return Pyrix64RunCpu(&Cpu, &Ram);
}

//prints the help text
static void Pyrix64PrintUsage(void)
{
    printf("PYriX64 pysmu emulator (%s)\n", PYSMVER);
    printf("\n");
    printf("usage: pysmu [\n");
    printf("    --fw program.bin\n");
    printf("]\n");
    printf("\n");
    printf("a fw needs to be specified\n");
}

int main(int Argc, char **Argv)
{
    if (Argc == 3 && strcmp(Argv[1], "--fw") == 0)
    {
        return Pyrix64RunFirmware(Argv[2]) ? 0 : 1;
    }

    Pyrix64PrintUsage();
    fprintf(stderr, "::  aerror: expected 'pysmu --fw <program.bin>'\n");

    return 1;
}
