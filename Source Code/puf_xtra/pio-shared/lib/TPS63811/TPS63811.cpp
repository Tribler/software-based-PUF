// This file is part of the software-based-PUF,
// https://github.com/Tribler/software-based-PUF
// Copyright (C) 2024 myndcryme
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

#include <math.h>
#include <Arduino.h>
#include <Wire.h>
#include <TPS63811.h>


/* elements are read only */
const struct BitIndex bx = {
  /* assign control register field bit index values (bits 4 and 7 NIL) */
	.range = {6},
	.enable = {5},
	.fpwm = {3},
	.rpwm = {2},
	.slew = {1,0},

	/* assign vout register field bit index values (bit 7 NIL)*/
	.vout1 = {6,5,4,3,2,1,0},
	.vout2 = {6,5,4,3,2,1,0}
};


/* pinMode() and digitalWrite() have no return value */
void tps63811_on(int pin)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  delay(1000);
}


void tps63811_off(int pin)
{
  digitalWrite(pin, LOW);
}


uint8_t tps63811_init(uint8_t val)
{
  uint8_t result = 0xFF;

  Wire.begin();   // init Wire lib and join I2C bus
  delay(1000);
  result = wire_write(CONTROL, val);    // default val sets range, fpwm, and slew bits, clears enable bit
  return result;
}


uint8_t wire_read(uint8_t addr, uint8_t *val)
{
  uint8_t result = 0xFF;    // init to error value

  Wire.beginTransmission(TPS63811_ADDR);
  Wire.write(addr);                         // select register
  result = Wire.endTransmission(false);     // do not generate STOP condition
  if (result == 0)
  {
    Wire.requestFrom(TPS63811_ADDR, 1);
    if (Wire.available() == 1)
    {
      *val = Wire.read();
    }
  }
  return result;    // return value is success/error indicator only
}


uint8_t wire_write(uint8_t addr, uint8_t val)
{
  uint8_t result = 0xFF;

  Wire.beginTransmission(TPS63811_ADDR);
  Wire.write(addr);                         // select register
  Wire.write(val);                          // que write register data
  result = Wire.endTransmission();          // write data
  return result;
}


uint8_t disable_output_converter()
{
  uint8_t mask = 0;
  uint8_t result = 0xFF;
  uint8_t value;

  mask |= 1 << bx.enable[0];
  result = wire_read(CONTROL, &value);
  if (result == 0)
  {
    value &= ~mask;
    result = wire_write(CONTROL, value);
  }
  return result;
}


uint8_t enable_output_converter()
{
  uint8_t mask = 0;
  uint8_t result = 0xFF;
  uint8_t value;

  mask |= 1 << bx.enable[0];
  result = wire_read(CONTROL, &value);
  if (result == 0)
  {
    value |= mask;
    result = wire_write(CONTROL, value);
  }
  return result;
}


/* Clears the bit(s) occupied by a single register field */
uint8_t register_clr_bit(uint8_t addr, const uint8_t field[], size_t n)
{
  uint8_t mask = 0;
  uint8_t result = 0xFF;
  uint8_t value;

  for (size_t i=0; i<n; i++)
  {
    mask |= 1 << field[i];
  }

  result = wire_read(addr, &value);
  if (result == 0)
  {
    value &= ~mask;
    result = wire_write(addr, value);
  }
  return result;
}


/* Sets the bit(s) occupied by a single register field.  Param4 (optional) specifies a value to assign for
 * multi-bit fields, otherwise any register bit occupied by the field is set HIGH.
 */
uint8_t register_set_bit(uint8_t addr, const uint8_t field[], size_t n, const uint8_t *val)
{
  uint8_t mask = 0;
  uint8_t result = 0xFF;
  uint8_t value;

  // if setting the RANGE bit, disable the output converter to avoid output voltage transients
  if ((n == 1) && (field[0] == bx.range[0]))    // check if setting RANGE bit
  {
    disable_output_converter();
  }

  // generate bitmask for field
  for (size_t i=0; i<n; i++)
  {
    mask |= 1 << field[i];
  }

  result = wire_read(addr, &value);
  if (result == 0)
  {
    if (val != nullptr)
    {
      /* assign specified value */
      value &= ~mask;   // clear field bits first
      value |= *val;    // assign field bits
    }
    else
    {
      /* set bits HIGH */
      value |= mask;
    }
    result = wire_write(addr, value);
  }

  // if RANGE bit was set, now re-enable the output converter
  if ((n == 1) && (field[0] == bx.range[0]))
  {
    enable_output_converter();
  }
  return result;
}


/* Generates register uint8_t value (increment multiplier) from vout float value (desired output voltage).  Valid return
 * value range is 0 to 0x7F because bit 7 of vout registers always return 0, thus a return > 0x7F indicates an error.
 */
uint8_t calc_vout_regval(float vout)
{
  uint8_t regval = 0xFF;    // vout register value, initialized to error
  uint8_t result;
  uint8_t ret = 0xFF;
  uint8_t bitmask = 1 << bx.range[0];   // 0b01000000

  float vout_frac;          // vout fractional portion
  float vout_int;           // vout integer portion
  float num_incr;           // number of increments in vout_frac
  float vout_valid;         // valid vout
  float vmin;               // min converter output voltage
  float vmax;               // max converter output voltage

  // read CONTROL register and get RANGE bit value
  ret = wire_read(CONTROL, &result);
  if (ret != 0)
  {
    return 0xFF;      // wire_read error
  }
  result &= bitmask;
  if (result == 0)    // range bit is cleared (LOW)
  {
    vmin = RL_MIN;
    vmax = RL_MAX;
  }
  else                // range bit is set (HIGH)
  {
    vmin = RH_MIN;
    vmax = RH_MAX;
  }

  // validate vout parameter
  if ((vout > vmax) || (vout < vmin))
  {
    return 0xFF;    // error (out of range)
  }

  vout_int = floor(vout);                               // vout integer portion
  vout_frac = vout - vout_int;                          // vout fractional portion
  num_incr = floor(vout_frac / VOUT_INCREMENT);         // number of increments in vout_frac
  vout_valid = vout_int + (VOUT_INCREMENT * num_incr);  // valid vout (rounds down to nearest increment)
  regval = (vout_valid - vmin) / VOUT_INCREMENT;        // valid vout = vout_min + regval*0.025
  return regval;
}