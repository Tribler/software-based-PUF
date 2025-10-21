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
#include "pufconst.h"

// init embedded instances
PUFBoard::PUFBoard() : xsram(), dac(), lvc1g125(), lms4684(), trans(){}   /*trans{}{} ?*/

// derived class instance getter
XSRAM& PUFBoard::getXSRAM(){
  return xsram;
}
/////////////////////////////////
// Adafruit MCP4725 DAC notes  //
/////////////////////////////////
/* Dn = input code (0 to 4095) */
/* Vout = VDD*(Dn/4096)        */
/* Dn = (Vout*4096)/VDD        */

// initialize select class scoped vars //
bool PUFBoard::calibrated = false;
uint8_t PUFBoard::vcca_conn_state = DISCONNECTED;                       // init Vcca connection state
uint32_t PUFBoard::dn_min = floor(VDD_MIN*DN_RES/VDD_SPL);              // Dn (input code) == vdd_min voltage
uint32_t PUFBoard::steps = DN_VDD - PUFBoard::dn_min;                   // available DAC steps between min and max voltages
uint32_t PUFBoard::step_us = floor(RAMP_MS*(1000.0f/PUFBoard::steps));  // step delay in microseconds

// initialize the PUF PCBA //
void PUFBoard::init_board()
{
  pin_in1 = get_pin_in1();
  pin_oe = get_pin_oe();
  pin_on = get_pin_on();

  // All pin I/O mode settings are relative to Arduino
  pinMode(pin_in1, OUTPUT);     // input1 pin (LMS4684)
  pinMode(pin_oe, OUTPUT);      // output enable pin (74LVC1G125)
  pinMode(pin_on, OUTPUT);      // TPS22917 ON pin
  digitalWrite(pin_in1, LOW);   // disconnect DAC to SRAM power path
  digitalWrite(pin_oe, HIGH);   // latch OE pin, Hi-Z / OFF state

  digitalWrite(pin_on, LOW);    // keep A-port off for translator isolation on init

  pin_cs = xsram.get_pin_cs();
  pin_hold = xsram.get_pin_hold();
  pin_miso = xsram.get_pin_miso();
  pin_mosi = xsram.get_pin_mosi();
  pin_sck = xsram.get_pin_sck();

  pinMode(pin_cs, OUTPUT);      // sram cs pin
  pinMode(pin_hold, OUTPUT);    // sram hold pin
  pinMode(pin_miso, OUTPUT);    // init miso as OUTPUT and change to INPUT prior to SPI operations
  pinMode(pin_mosi, OUTPUT);    // sram mosi
  pinMode(pin_sck, OUTPUT);     // sram clock

  // sram pins should not exceed VCC+0.3V //
  // CS and HOLD must be pulled HIGH after power ramp //
  // note all sram pins are isolated until translator A-port is powered //

  // the following assignments only affect translator side B pins for now
  digitalWrite(pin_cs, LOW);      // unpowered A-port ==> SRAM is isolated
  digitalWrite(pin_hold, LOW);    //
  digitalWrite(pin_mosi, LOW);
  digitalWrite(pin_miso, LOW);    // drive LOW for now, change miso to INPUT before SPI operations
}

// get Vcca connection state //
uint8_t PUFBoard::get_vcca_state(){
  return vcca_conn_state;
}

// set Vcca connection state //
void PUFBoard::set_vcca_state(uint8_t state){
  vcca_conn_state = state;
}

// manages pins appropriately and powers Vcca (removes A/B port isolation) //
void PUFBoard::vcca_connect()
{
  pinMode(pin_miso, INPUT);       // *MUST* change miso back to INPUT prior to connecting

  // these pin writes affect B-side only until Vcca is connected below
  digitalWrite(pin_cs, HIGH);     // SPI deselect
  digitalWrite(pin_hold, HIGH);
  digitalWrite(pin_on, HIGH);     // Vcca connect, removing A/B isolation
  set_vcca_state(CONNECTED);
  // A-side now connected with B-side, ready for SPI communication
}

// manages pins and removes power from Vcca for Vcc disconnect (A/B port isolation) //
void PUFBoard::vcca_disconnect()
{
  pinMode(pin_miso, OUTPUT);      // *MUST* change miso to OUTPUT when driving LOW (for 0V)

  // make sure all I/Os are LOW prior to Vcc disconnect
  digitalWrite(pin_mosi, LOW);
  digitalWrite(pin_sck, LOW);
  digitalWrite(pin_hold, LOW);
  digitalWrite(pin_cs, LOW);      // overrides CS (side B) ~10k pull-up until Vcc disconnect
  digitalWrite(pin_on, LOW);      // cut power to Vcca ==> Vcc disconnect
  delayMicroseconds(500);         // gives TPS22917 QOD time to discharge (fall time should be ~ 165 us)
  set_vcca_state(DISCONNECTED);

  // side A is now isolated with all inputs LOW
  // any pin writes that follow affect side B only until portA Vcca reconnect

  //digitalWrite(pin_cs, HIGH);     // optional
  //digitalWrite(pin_hold, HIGH);   // optional
}

// uses DAC instead of Arduino output pin for power
void PUFBoard::sram_pwr_off()
{
  pinMode(pin_miso, OUTPUT);      // *MUST* set MISO as OUTPUT prior to driving LOW for 0V level
  digitalWrite(pin_cs, LOW);      // || set I/Os LOW prior to Vcc disconnect
  digitalWrite(pin_hold, LOW);    // ||
  digitalWrite(pin_mosi, LOW);    // ||
  digitalWrite(pin_miso, LOW);    // ||
  digitalWrite(pin_sck, LOW);     // \/

  digitalWrite(pin_oe, HIGH);     // latches OE pin, Hi-Z / OFF state (pwr off from fast ramp)
  dac.setVoltage(0, false);       // set DAC voltage to 0V
  digitalWrite(pin_in1, LOW);     // disconnects DAC to SRAM power path
}

// legacy power on replacement, sets voltage to DAC VDD input (max voltage) - no controlled ramp
void PUFBoard::sram_pwr_on()
{
  digitalWrite(pin_oe, HIGH);         // should already be HIGH (OFF state)
  digitalWrite(pin_in1, HIGH);        // drive IN1 HIGH, closing NO1 and connecting DAC to SRAM power path
  dac.setVoltage(DN_VDD, false);      // DN_VDD == 4095 == max Dn input code (results in voltage VDD)
  vcca_connect();                     // remove sram isolation
}

void PUFBoard::calibrate_step_delay()
{
  uint32_t t0, t1, et;                    // begin, end, elapsed time

  dac.setVoltage(dn_min, false);          // set voltage with dn_min (equal to vdd_min INPUT CODE)
  delay(HOLD_MS);
  t0 = micros();                          // || for calibration
  dac.setVoltage(dn_min+1, false);        // ||
  delayMicroseconds(step_us);             // ||
  t1 = micros();                          // \/
  et = t1-t0;                             // microseconds elapsed
  dac.setVoltage(0, false);               // take it down
  step_us -= et - step_us;                // adjust the step delay (removes overhead)
  calibrated = true;
}

// returns 0 on success, 1 on warning //
int PUFBoard::sram_slow_on()
{
  uint32_t dn;
  uint32_t t0, t1, et;                  // begin, end, elapsed time

  if (!calibrated)                      // make sure the step is calibrated for accurate ramp
  {
    calibrate_step_delay();
  }

  digitalWrite(pin_in1, HIGH);          // drive IN1 HIGH, closing NO1 and connecting DAC to SRAM power path
  dac.setVoltage(dn_min, false);        // set initial voltage with dn_min (vdd_min *input code*)
  delay(HOLD_MS);                       // hold min voltage for specified ms
  t0 = millis();
  for (dn=dn_min; dn<DN_VDD; dn++)      // voltage ramp loop
  {
    dac.setVoltage(dn, false);
    delayMicroseconds(step_us);
  }
  vcca_connect();                       // slow ramp complete, remove Vcc isolation
  t1 = millis();
  et = t1-t0;                           // elapsed time used to validate actual vs. expected ramp time
  if (((double)abs(RAMP_MS - et)/(double)RAMP_MS) > THRESH)
  {
    return 1;   // warning: actual ramp time deviates from expected beyond threshold
  }
  return 0;
}

void PUFBoard::sram_fast_on()
{
  digitalWrite(pin_oe, LOW);    // enable output, fast ramp on
  delayMicroseconds(1000);      // short delay so VCC can settle before pulling HOLD up
  digitalWrite(pin_hold, HIGH);
}

void PUFBoard::sram_fast_off()
{
  digitalWrite(pin_hold, LOW);  // MUST first set hold pin LOW when SRAM is powered off
  digitalWrite(pin_oe, HIGH);
}

bool PUFBoard::dac_begin(uint8_t addr)
{
  bool b = dac.begin(addr);
  TWBR = 12;          // 400 KHz operation

  /* write non-volatile value to EEPROM (preferred) for persistent start up of 0V, or uncomment next line */
  dac.setVoltage(0, false);
  return b;
}

// replaces pwr on/off toggle for new bit selection method //
bool PUFBoard::dac_set_voltage(uint16_t dn, bool b)
{
  bool val = dac.setVoltage(dn, b);
  return val;
}
