// XSRAM.cpp - the derived class 'XSRAM' inherits from
// the base SRAM class written by Ade Setyawan.  Adds functionality
// for alternate bit selection method, while preserving original code.
//
// This file is part of software-based-PUF,
// https://github.com/Tribler/software-based-PUF
// Copyright (C) 2024 myndcryme.
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

Adafruit_MCP4725 dac;

XSRAM::XSRAM() : SRAM(){}

// override
void set_pin_power(){}    // using DAC instead

/* modified to use DAC instead of Arduino output pin */
void XSRAM::turn_off()
{
  SPI.end();

  pinMode(pin_miso, OUTPUT);
  pinMode(pin_mosi, OUTPUT);
  pinMode(pin_sck, OUTPUT);

  digitalWrite(pin_miso, LOW);
  digitalWrite(pin_mosi, LOW);
  digitalWrite(pin_sck, LOW);
  dac.setVoltage(0, false);         // set DAC voltage to 0V
  digitalWrite(pin_hold, LOW);
  digitalWrite(pin_cs, HIGH);         // changed from LOW
}

/* modified to use DAC - sets voltage to DAC VDD input (max voltage) */
void XSRAM::turn_on()
{
  pinMode(pin_cs, OUTPUT);
  pinMode(pin_hold, OUTPUT);
  digitalWrite(pin_cs, LOW);
  digitalWrite(pin_hold, HIGH);
  dac.setVoltage(4095, false);    // 4095 is max Dn input code (results in voltage VDD)

  SPI.begin();
}

/* call prior to voltage ramp routine */
void XSRAM::config_pre_ramp()
{
  pinMode(pin_cs, OUTPUT);
  pinMode(pin_hold, OUTPUT);
  digitalWrite(pin_cs, LOW);
  digitalWrite(pin_hold, HIGH);

  SPI.begin();
}

void XSRAM::dac_begin(uint8_t addr)
{
  dac.begin(addr);
  TWBR = 12;          // 400 KHz operation

  /* write non-volatile value to EEPROM for persistent start up of 0V, or uncomment next line */
  // dac.setVoltage(0, false);
}

void XSRAM::dac_set_voltage(uint16_t dn, bool b)
{
  dac.setVoltage(dn, b);
}