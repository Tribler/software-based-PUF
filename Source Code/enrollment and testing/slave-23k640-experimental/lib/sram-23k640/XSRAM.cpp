// XSRAM.cpp - the derived class 'XSRAM' inherits from
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

#include "XSRAM.h"

#define SRAM_23K640   // using 23K640 for ramp method

// *note - 23K640 instruction set/format (READ, WRITE, RDSR, WRSR) is the same as 23LC1024

XSRAM::XSRAM() : SRAM(){
#ifdef SRAM_23K640  // re-assign vars from 23LC1024 (SRAM class defaults)
  maxram = 32768;   // bytes in 23K640 SRAM
  maxpage = 1024;   // pages in 23K640 SRAM
#endif
}

// hide, since using DAC instead
void XSRAM::set_pin_power(){}
void XSRAM::turn_off(){}
void XSRAM::turn_on(){}

uint8_t XSRAM::get_pin_cs() const{
  return pin_cs;
}

uint8_t XSRAM::get_pin_hold() const{
  return pin_hold;
}

uint8_t XSRAM::get_pin_power() const{
  return pin_power;
}

uint8_t XSRAM::get_pin_mosi() const{
  return pin_mosi;
}

uint8_t XSRAM::get_pin_miso() const{
  return pin_miso;
}

uint8_t XSRAM::get_pin_sck() const{
  return pin_sck;
}
