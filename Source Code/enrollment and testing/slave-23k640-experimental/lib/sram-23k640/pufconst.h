// pufconst.h is for 23k640/pufboard constants.
// This file is part of software-based-PUF,
// https://github.com/Tribler/software-based-PUF
// Copyright (C) 2025 myndcryme.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef PUFCONST_H
#define PUFCONST_H

// DAC constants
#define DAC_ADDR 0x62   // ADAFRUIT_MCP4725_I2C_ADDR
#define DN_RES 4096     // DAC input code resolution (12 bits)
#define DN_VDD 4095     // VDD input code, equals supply voltage input code
#define VDD 3.300f      // DAC supply voltage
#define VDD_MIN 0.58f   // DAC begin ramp voltage
#define VDD_SPL VDD     // DAC end ramp voltage, same as supply voltage
#define VSS 0.0f        // ground reference

// Arduino pin constants
#define HOLD_PIN 11
#define MISO_PIN 50
#define MOSI_PIN 51
#define SCK_PIN 52
#define SD_CS_PIN 7       // chip select pin for SD card breakout
#define SRAM_CS_PIN 10    // chip select pin for SRAM IC
#define OE_PIN 3          // OE (74LVC1G125 output enable)
#define IN1_PIN 8         // IN1 (LMS4684 input1)
#define ON_PIN 5          // ON (TPS22917), controls Vcca for 74LXC2T45 translators

// Ramp constants
#define RAMP_MS 1000      // slow ramp time setting (ms)
#define HOLD_MS 1000      // hold time setting (ms)
#define THRESH 0.05       // slow ramp time deviation threshold

#endif