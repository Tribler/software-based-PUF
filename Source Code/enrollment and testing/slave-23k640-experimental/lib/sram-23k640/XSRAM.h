// XSRAM.h - the derived class 'XSRAM' inherits from
// the base SRAM class written by Ade Setyawan.  Adds functionality
// for alternate bit selection method, while preserving original code.
//
// This file is part of software-based-PUF,
// https://github.com/Tribler/software-based-PUF
// Copyright (C) 2024, 2025 myndcryme.
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

#ifndef XSRAM_H
#define XSRAM_H

#include "SRAM.h"

// XSRAM extends base SRAM class for new bit selection method
class XSRAM : public SRAM {
public:
  XSRAM();    // constructor

  // hide these since they won't work powering the SRAM via DAC
  void set_pin_power();
  void turn_off();
  void turn_on();

  // add necessary getters
  uint8_t get_pin_cs() const;
  uint8_t get_pin_hold() const;
  uint8_t get_pin_power() const;
  uint8_t get_pin_mosi() const;
  uint8_t get_pin_miso() const;
  uint8_t get_pin_sck() const;
};

#endif