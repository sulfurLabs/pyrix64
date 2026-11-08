#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2026 sulfurLabs
#
# PROJECT: pyrix64 - pysmu
# FILE: Makefile
# DESC.: Makefile
#
# This file is part of pyrix64.
#
# pyrix64 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# pyrix64 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the LICENSE file for the full license text.
# You can also find the GPL-3.0 license at:
# https://www.gnu.org/licenses/gpl-3.0.html
#

include ../../common.mk

TARGET := $(BUILD_DIR)/pysmu/pysmu
SOURCES := main.c cpu.c instr.c ram.c exec/bin.c
CPPFLAGS += -I$(ROOT_DIR)/pyrix/pysmu

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
