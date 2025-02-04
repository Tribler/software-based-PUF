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

#ifndef PUFBOARD_H
#define PUFBOARD_H

#include "XSRAM.h"
#include <Adafruit_MCP4725.h>

class PUFBoard{
private:
  // board class scoped pin variables
  uint8_t pin_cs;
  uint8_t pin_hold;
  uint8_t pin_miso;
  uint8_t pin_mosi;
  uint8_t pin_sck;
  uint8_t pin_in1;
  uint8_t pin_oe;
  uint8_t pin_vcc;

  // nested classes for puf board IC pin config and function
  class Buffer_74LVC1G125{    // tri state buffer
  private:
    uint8_t pin_oe;     // 74LVC1G125 output enable pin
    uint8_t pin_vcc;    // 74LVC1G125 supply voltage pin

  public:
    uint8_t get_oe() const{
      return pin_oe;
    }
    uint8_t get_vcc() const{
      return pin_vcc;
    }
    void set_oe(uint8_t pin){
      pin_oe = pin;
    }
    void set_vcc(uint8_t pin){
      pin_vcc = pin;
    }
  };

  class Switch_LMS4684{   // analog switch
  private:
    uint8_t pin_in1;      // input1 pin

  public:
    uint8_t get_in1() const{
      return pin_in1;
    }
    void set_in1(uint8_t pin){
      pin_in1 = pin;
    }
  };

  XSRAM xsram;                  // embedded instance
  Adafruit_MCP4725 dac;         // embedded instance
  Buffer_74LVC1G125 lvc1g125;   // buffer embedded instance
  Switch_LMS4684 lms4684;       // analog switch embedded instance

public:
  PUFBoard();
  XSRAM& getXSRAM();          // class instance getter

  // public getters for nested class private vars
  uint8_t get_pin_oe() const{
    return lvc1g125.get_oe();
  }
  uint8_t get_pin_vcc() const{
    return lvc1g125.get_vcc();
  }
  uint8_t get_pin_in1() const{
    return lms4684.get_in1();
  }

  // public setters
  void set_pin_oe(uint8_t pin){
    lvc1g125.set_oe(pin);
  }
  void set_pin_vcc(uint8_t pin){
    lvc1g125.set_vcc(pin);
  }
  void set_pin_in1(uint8_t pin){
    lms4684.set_in1(pin);
  }

  void init_board();
  void config_slow_ramp();
  void config_fast_ramp();
  void sram_fast_on();
  void sram_fast_off();
  void sram_power_off();
  void sram_power_on();
  bool dac_begin(uint8_t addr);
  bool dac_set_voltage(uint16_t dn, bool b);
};

#endif