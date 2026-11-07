#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2026 sulfurLabs
#
# PROJECT: pyrix64
# FILE: Makefile
# DESC.: Build configuration
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

include common.mk

.PHONY: all pysmu pyx clean

all: pysmu pyx

pysmu:
	$(MAKE) -C pyrix/pysmu
	./build/pysmu/pysmu

pyx:
	$(MAKE) -C pyrix/pyx
	./build/pyx/pyx

clean:
	$(MAKE) -C pyrix/pysmu clean
	$(MAKE) -C pyrix/pyx clean
