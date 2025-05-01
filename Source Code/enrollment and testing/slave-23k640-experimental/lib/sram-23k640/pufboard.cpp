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
PUFBoard::PUFBoard() : xsram(), dac(), lvc1g125(), lms4684(), txb0106(){}

// derived class instance getter
XSRAM& PUFBoard::getXSRAM(){
  return xsram;
}

void PUFBoard::init_board()
{
  pin_in1 = get_pin_in1();
  pin_oe = get_pin_oe();
  pin_oe_txb = get_pin_oe_txb();
  //pin_vcc = get_pin_vcc();

  pinMode(pin_in1, OUTPUT);     // input1 pin (LMS4684) - pin mode OUTPUT for *Arduino* is correct
  pinMode(pin_oe, OUTPUT);      // output enable pin (74LVC1G125)
  pinMode(pin_oe_txb, OUTPUT);  // txb0601 output enable pin
  //pinMode(pin_vcc, OUTPUT);
  digitalWrite(pin_in1, LOW);   // disconnect DAC to SRAM power path
  digitalWrite(pin_oe, HIGH);   // latch OE pin, Hi-Z / OFF state
  digitalWrite(pin_oe_txb, LOW);   // LOW places all shifter I/Os in a high impedance state (OFF state)

  pin_cs = xsram.get_pin_cs();
  pin_hold = xsram.get_pin_hold();
  pin_miso = xsram.get_pin_miso();
  pin_mosi = xsram.get_pin_mosi();
  pin_sck = xsram.get_pin_sck();

  pinMode(pin_cs, OUTPUT);      // sram cs pin
  pinMode(pin_hold, OUTPUT);    // sram hold pin
}

void PUFBoard::txb_enable()
{
  digitalWrite(pin_oe_txb, HIGH);
}

void PUFBoard::txb_disable()
{
  digitalWrite(pin_oe_txb, LOW);    // all TXB I/Os in a high impedance state (OFF state)
}

// uses DAC instead of Arduino output pin for power
void PUFBoard::sram_power_off()
{
  SPI.end();

  pinMode(pin_miso, OUTPUT);
  pinMode(pin_mosi, OUTPUT);
  pinMode(pin_sck, OUTPUT);

  digitalWrite(pin_oe, HIGH);         // latches OE pin, Hi-Z / OFF state
  digitalWrite(pin_in1, LOW);         // disconnects DAC to SRAM power path
  dac.setVoltage(0, false);           // set DAC voltage to 0V
  digitalWrite(pin_miso, LOW);
  digitalWrite(pin_mosi, LOW);        //  || set all inputs LOW when SRAM is powered off
  digitalWrite(pin_sck, LOW);         //  ||
  digitalWrite(pin_hold, LOW);        //  ||
  digitalWrite(pin_cs, LOW);          //  \/
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

  /* temporarily keep the HOLD pin LOW (or possibly float?) during VCC ramp since input pins should not exceed VCC+0.3V
   * but HOLD must be pulled HIGH after power ramp - also note HIGH is 3.3V for 23K640 so signal must be translated
   */

  digitalWrite(pin_hold, LOW);
  SPI.begin();
}

// fast ramp CONFIG
void PUFBoard::config_fast_ramp()
{
  digitalWrite(pin_in1, LOW);   // disconnect DAC VOUT from SRAM VCC
  digitalWrite(pin_oe, HIGH);   // keep OE HIGH until actually ramp is executed
  digitalWrite(pin_cs, LOW);

  /* temporarily keep the HOLD pin LOW (or possibly float?) during VCC ramp since input pins should not exceed VCC+0.3V
   * but HOLD must be pulled HIGH after power ramp completes - also note HIGH is 3.3V for 23K640 so signal must be
   * translated
   */

  digitalWrite(pin_hold, LOW);
  SPI.begin();
}

void PUFBoard::sram_fast_on()
{
  digitalWrite(pin_oe, LOW);    // enable output, fast ramp on
  delayMicroseconds(1000);      // short delay so VCC can settle before pulling HOLD up
  digitalWrite(pin_hold, HIGH);
}

void PUFBoard::sram_fast_off()
{
  digitalWrite(pin_hold, LOW);    // must first set hold pin LOW when SRAM is powered off
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
