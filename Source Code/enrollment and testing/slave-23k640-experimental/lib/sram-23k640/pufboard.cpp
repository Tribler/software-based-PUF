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

#include "pufboard.h"

// init embedded instances
PUFBoard::PUFBoard() : xsram(), dac(), lvc1g125(), lms4684(){}

// derived class instance getter
XSRAM& PUFBoard::getXSRAM(){
  return xsram;
}

void PUFBoard::init_board()
{
  pin_in1 = get_pin_in1();
  pin_oe = get_pin_oe();
  //pin_vcc = get_pin_vcc();

  pinMode(pin_in1, OUTPUT);     // input1 pin (LMS4684) - pin mode OUTPUT for *Arduino* is correct
  pinMode(pin_oe, OUTPUT);      // output enable pin (74LVC1G125)
  //pinMode(pin_vcc, OUTPUT);
  digitalWrite(pin_in1, LOW);   // disconnect DAC to SRAM power path
  digitalWrite(pin_oe, HIGH);   // latch OE pin, Hi-Z / OFF state

  pin_cs = xsram.get_pin_cs();
  pin_hold = xsram.get_pin_hold();
  pin_miso = xsram.get_pin_miso();
  pin_mosi = xsram.get_pin_mosi();
  pin_sck = xsram.get_pin_sck();

  pinMode(pin_cs, OUTPUT);      // sram cs pin
  pinMode(pin_hold, OUTPUT);    // sram hold pin
}

// uses DAC instead of Arduino output pin for power
void PUFBoard::sram_power_off()
{
  SPI.end();

  digitalWrite(pin_oe, HIGH);         // latches OE pin, Hi-Z / OFF state
  digitalWrite(pin_in1, LOW);         // disconnects DAC to SRAM power path
  dac.setVoltage(0, false);           // set DAC voltage to 0V
  digitalWrite(pin_miso, LOW);
  digitalWrite(pin_mosi, LOW);
  digitalWrite(pin_sck, LOW);
  digitalWrite(pin_hold, LOW);
  digitalWrite(pin_cs, HIGH);         // de-select
}

// sets voltage to DAC VDD input (max voltage)
void PUFBoard::sram_power_on()
{
  digitalWrite(pin_in1, HIGH);
  digitalWrite(pin_oe, HIGH);
  digitalWrite(pin_cs, LOW);
  digitalWrite(pin_hold, HIGH);
  dac.setVoltage(4095, false);    // 4095 is max Dn input code (results in voltage VDD)

  SPI.begin();
}

// slow ramp CONFIG
void PUFBoard::config_slow_ramp()
{
  digitalWrite(pin_in1, HIGH);    // drive IN1 HIGH, closing NO1, connecting DAC to SRAM power path
  digitalWrite(pin_oe, HIGH);     // drive OE HIGH, ensuring Hi-Z / OFF state
  digitalWrite(pin_cs, LOW);
  digitalWrite(pin_hold, HIGH);

  SPI.begin();
}

// fast ramp CONFIG
void PUFBoard::config_fast_ramp()
{
  digitalWrite(pin_in1, LOW);   // disconnect DAC VOUT from SRAM VCC
  digitalWrite(pin_oe, HIGH);   // keep OE HIGH until actually ramp is executed
  digitalWrite(pin_cs, LOW);
  digitalWrite(pin_hold, HIGH);

  SPI.begin();
}

void PUFBoard::sram_fast_on()
{
  digitalWrite(pin_oe, LOW);    // enable output, fast ramp on
}

void PUFBoard::sram_fast_off()
{
  digitalWrite(pin_oe, HIGH);
}

bool PUFBoard::dac_begin(uint8_t addr)
{
  bool b = dac.begin(addr);
  TWBR = 12;          // 400 KHz operation
  return b;

  /* write non-volatile value to EEPROM for persistent start up of 0V, or uncomment next line */
  // dac.setVoltage(0, false);
}

// replaces pwr on/off toggle for new bit selection method
bool PUFBoard::dac_set_voltage(uint16_t dn, bool b)
{
  bool val = dac.setVoltage(dn, b);
  return val;
}
