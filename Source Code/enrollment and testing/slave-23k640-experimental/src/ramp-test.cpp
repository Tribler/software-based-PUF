// ramp-test.cpp is a WIP test file for experimentation with
// an alternative bit selection method for SRAM PUFs.
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

#include <math.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <XSRAM.h>      // includes Adafruit_MCP4725.h

#define DAC_ADDR 0x62   // ADAFRUIT_MCP4725_I2C_ADDR
#define DN_RES 4096     // DAC input code resolution (12 bits)
#define DN_VDD 4095     // Dn (input code) == supply voltage, VDD
#define VDD 3.300       // DAC supply voltage
#define VSS 0.0         // ground reference

#define HOLD_PIN 11
#define MISO_PIN 50
#define MOSI_PIN 51
#define SCK_PIN 52
#define SD_CS_PIN 7     // chip select pin for SD card breakout
#define SRAM_CS_PIN 10  // chip select pin for SRAM IC

const char HEX_TABLE[] = "0123456789ABCDEF";

XSRAM xsram;

uint8_t result_page[32] = {'\0'};
size_t rp_size = sizeof(result_page);

void read_page(uint32_t address)
{
  memset(result_page, '\0' , rp_size);
  xsram.read_page(address, result_page);
}

void uint8_print_hex(uint8_t val, bool sep)
{
  char result[3] = {'\0'};

  result[0] = HEX_TABLE[val >> 4];
  result[1] = HEX_TABLE[val & 0xF];
  Serial.print(result);
  if (sep)
    Serial.print(" ");
}

/* NOTE - MCP4725 slew rate is 0.55 V/us, thus the fastest possible ramp with
 * this DAC is around 5 to 6 microseconds when ramping from .58 V to 3.3 V.  To
 * mimic researcher's fast ramp (~ 7ns) we can probably get close building a gated
 * ramp circuit.  Powering an Arduino output pin ramps quite slow (~ 1us).
 */

void setup()
{
  File myFile;              // SD card file
  const char *filename = "test.txt";
  float vdd_min = 0.58;    // ramp begin voltage
  float vdd_spl = VDD;     // ramp end voltage

  // Dn = input code (0 to 4095)
  // Vout = VDD*(Dn/4096)
  // Dn = (Vout*4096)/VDD

  uint32_t dn;          // input code
  uint32_t dn_min = floor((vdd_min*DN_RES)/vdd_spl);    // Dn (input code) == vdd_min voltage
  uint32_t steps = DN_VDD - dn_min;          // available DAC steps between min and max voltages
  uint32_t hold_ms = 1000;        // hold time in ms
  uint32_t ramp_ms = 1000;        // ramp time in ms

  // TODO:  account for loop and operations time for an accurate step value
  uint32_t step_usec = floor((ramp_ms*1000)/steps);

  xsram = XSRAM();

  xsram.set_pin_cs(SRAM_CS_PIN);
  xsram.set_pin_hold(HOLD_PIN);
  xsram.set_pin_mosi(MOSI_PIN);
  xsram.set_pin_miso(MISO_PIN);
  xsram.set_pin_sck(SCK_PIN);

  Serial.begin(115200);
  delay(500);
  Serial.println("\ninitializing DAC");
  xsram.dac_begin(DAC_ADDR);    // initialize DAC and join I2C bus
  delay(500);
  Serial.print("init sd card...");
  if (!SD.begin(SD_CS_PIN))
  {
    Serial.println("init failed");
    while(1);    // wait forever till sd bring up
  }
  Serial.println("done");
  delay(1000);

  Serial.println("set min voltage, hold, then slow ramp to supply voltage");
  xsram.config_pre_ramp();                 // configure pins before DAC output
  xsram.dac_set_voltage(dn_min, false);    // set initial voltage with vdd_min input code
  delay(hold_ms);                          // hold min voltage for specified ms
  for (dn=dn_min; dn<DN_VDD; dn++)         // voltage ramp loop
  {
    xsram.dac_set_voltage(dn, false);
    delayMicroseconds(133);               // results in approximately a one sec ramp time
    // delayMicroseconds(step_usec);      // TODO: step_usec needs a correction factor to work properly
  }

  // read sram test
  Serial.println("read page test");

  // 23LC1024 4096 total pages, let's test read some
  // TODO: add 23K640 SRAM
  for (int n=0; n<8; n++)
  {
    uint32_t addr = n*32;

    Serial.print(addr, HEX);
    Serial.print(":  ");
    read_page(addr);
    for (int i=0; i<32; i++)
    {
      uint8_print_hex(result_page[i], true);
    }
    Serial.println("");
  }
  xsram.turn_off();

  myFile = SD.open(filename, FILE_WRITE);
  if (myFile)
  {
    // TODO: write test data to sd or save on host
    myFile.close();
  }
  else
  {
    Serial.println("error opening file");
  }
}

void loop()
{
  Serial.print(".");
  delay(1000);
}