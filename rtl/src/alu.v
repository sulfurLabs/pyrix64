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

module pyrix64ALU (
    input wire [63:0] a,
    input wire [63:0] b,
    input wire [1:0] Operation,

    output reg [63:0] Result
);
    always @(*) begin
        case (Operation)
            2'b00: Result = a + b;
            2'b01: Result = a - b;
            2'b10: Result = a & b;
            2'b11: Result = a | b;

            default: Result = 64'b0;
        endcase
    end
endmodule /* pyrix64ALU */
