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
  // class scoped constants
  static const uint8_t GRP_COUNT = 3;     // number of translators in the trans group
  static const uint8_t CONNECTED = 0;
  static const uint8_t DISCONNECTED = 1;

  // class scoped vars (init in src file)
  static bool calibrated;                 // DAC calibration flag
  static uint8_t vcca_conn_state;         // Vcca connection state flag
  static uint32_t dn_min;                 // Dn (input code) == vdd_min voltage
  static uint32_t steps;                  // available DAC steps between min and max voltages
  static uint32_t step_us;                // step delay in microseconds

  // board class scoped pin variables
  uint8_t pin_cs;       // SRAM
  uint8_t pin_hold;     // ||
  uint8_t pin_miso;     // ||
  uint8_t pin_mosi;     // ||
  uint8_t pin_sck;      // \/

  uint8_t pin_in1;              // LMS switch in1
  uint8_t pin_oe;               // LVC buffer oe
  uint8_t pin_vcc;              // LVC buffer vcc
  uint8_t pin_on;               // TPS switch on
  uint8_t pin_b1[GRP_COUNT];    // LXC translator b1
  uint8_t pin_b2[GRP_COUNT];    // LXC translator b2


  // nested classes for puf board IC pin config and function //
  class Buffer_74LVC1G125{    // tri state buffer
  private:
    uint8_t pin_oe;     // 74LVC1G125 output enable pin

  public:
    uint8_t get_oe() const{
      return pin_oe;
    }
    void set_oe(uint8_t pin){
      pin_oe = pin;
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

  // voltage translator SN74LXC2T45 has no OE function     //
  // use Vccx disconnect feature (supply < 100 mV) instead //
  // recommended to pull I/Os down prior to floating Vccx  //
  // high (B-port) always powered, low (A-port) switched   //

  class Trans_74LXC2T45{   // SN74LXC2T45
  private:
    // only B1, B2 pins are adjustable //
    // A1, A2, Vcca, Vccb, DIR, GND are hard-wired //
    uint8_t pin_b1;
    uint8_t pin_b2;

  public:
    uint8_t get_b1() const{
      return pin_b1;
    }
    uint8_t get_b2() const{
      return pin_b2;
    }
    void set_b1(uint8_t pin){
      pin_b1 = pin;
    }
    void set_b2(uint8_t pin){
      pin_b2 = pin;
    }
  };

  class Switch_TPS22917{   // TPS22917 for switching 74LXC2T45 Vcca power
  private:
    uint8_t pin_on;    // ON pin is active HIGH, don't float

  public:
    uint8_t get_on() const{
      return pin_on;
    }
    void set_on(uint8_t pin){
      pin_on = pin;
    }
  };


  XSRAM xsram;                        // embedded instance
  Adafruit_MCP4725 dac;               // dac embedded instance
  Buffer_74LVC1G125 lvc1g125;         // buffer embedded instance
  Switch_LMS4684 lms4684;             // analog switch embedded instance
  Trans_74LXC2T45 trans[GRP_COUNT];   // create a group of embedded translator instances
  Switch_TPS22917 tps;                // translators A-port power switch instance


public:
  PUFBoard();
  XSRAM& getXSRAM();              // class instance getter

  // public getters for nested class private vars
  uint8_t get_pin_oe() const{
    return lvc1g125.get_oe();
  }
  uint8_t get_pin_in1() const{
    return lms4684.get_in1();
  }
  uint8_t get_pin_on() const{
    return tps.get_on();
  }
  uint8_t get_pin_b1(uint8_t index) const{
    return trans[index].get_b1();
  }
  uint8_t get_pin_b2(uint8_t index) const{
    return trans[index].get_b2();
  }

  // setters for nested class private vars
  void set_pin_oe(uint8_t pin){
    lvc1g125.set_oe(pin);
  }
  void set_pin_in1(uint8_t pin){
    lms4684.set_in1(pin);
  }
  void set_pin_on(uint8_t pin){
    tps.set_on(pin);
  }
  void set_pin_b1(uint8_t index, uint8_t pin){
    trans[index].set_b1(pin);
  }
  void set_pin_b2(uint8_t index, uint8_t pin){
    trans[index].set_b2(pin);
  }

  bool dac_begin(uint8_t addr);
  bool dac_set_voltage(uint16_t dn, bool b);
  uint8_t get_vcca_state();
  void set_vcca_state(uint8_t state);

  void init_board();            // initialize puf pcba
  int sram_slow_on();           // execute slow ramp
  void sram_fast_on();          // 74LVC1G125 enable output (sets OE pin LOW)
  void sram_fast_off();         // 74LVC1G125 disable output (sets OE pin HIGH)
  void sram_pwr_off();          // complete SRAM power off (for both slow / fast / legacy)
  void sram_pwr_on();           // legacy power on/off replacement (uses DAC)
  void vcca_connect();          // apply power to LXC A-port supply (all translators)
  void vcca_disconnect();       // remove power from LXC A-port supply (all)
  void calibrate_step_delay();  // adjusts step delay for accurate slow ramp time
};

#endif