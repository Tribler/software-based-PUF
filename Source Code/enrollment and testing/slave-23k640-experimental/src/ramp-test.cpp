// ramp-test.cpp is a WIP test file for experimentation with
// an alternative bit selection method for SRAM PUFs.
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

#include <math.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <pufboard.h>
#include "pufconst.h"               // constants for DAC, Arduino pins, etc.

PUFBoard board;                     // PUF breakout board instance
XSRAM& xsram = board.getXSRAM();    // allow access XSRAM instance directly

const char HEX_TABLE[] = "0123456789ABCDEF";

void uint8_print_hex(uint8_t val, bool sep)
{
  char result[3] = {'\0'};

  result[0] = HEX_TABLE[val >> 4];
  result[1] = HEX_TABLE[val & 0xF];
  Serial.print(result);
  if (sep)
    Serial.print(" ");
}

void setup()
{
  // set pin assignments
  board.set_pin_in1(IN1_PIN);       // puf board IC LMS4684 in1 pin
  board.set_pin_oe(OE_PIN);         // puf board IC 74LVC1G125 oe pin
  board.set_pin_on(ON_PIN);
  board.set_pin_b1(0, HOLD_PIN);    // sets translator[0] b1 pin
  board.set_pin_b2(0, SCK_PIN);     // sets translator[0] b2 pin
  board.set_pin_b1(1, MOSI_PIN);    // sets translator[1] b1 pin
  board.set_pin_b2(1, SRAM_CS_PIN); // sets translator[1] b2 pin
  board.set_pin_b1(2, MISO_PIN);    // sets translator[2] b1 pin
  board.set_pin_b2(2, 0xFF);        // translator[2] b2 pin is unused

  // set sram pins prior to board init
  xsram.set_pin_cs(SRAM_CS_PIN);
  xsram.set_pin_hold(HOLD_PIN);
  xsram.set_pin_mosi(MOSI_PIN);
  xsram.set_pin_miso(MISO_PIN);
  xsram.set_pin_sck(SCK_PIN);

  // then initialize the board
  board.init_board();
  delay(500);

  Serial.begin(115200);
  delay(500);
  Serial.println("\ninitializing DAC");
  board.dac_begin(DAC_ADDR);    // initialize DAC and join I2C bus
  delay(500);
  SPI.begin();

//  File myFile;                              // SD card file
//  const char *filename = "test.txt";
//
//  Serial.print("init sd card...");
//  if (!SD.begin(SD_CS_PIN))
//  {
//    Serial.println("sd init failed");
//    delay(1000);
//    while(1);    // wait forever till sd bring up
//  }
//  Serial.println("done");

  ////////////////////////////
  // slow ramp test routine //
  ////////////////////////////

  Serial.println("calibrating DAC step delay");
  board.calibrate_step_delay();
  delay(1000);
  Serial.println("executing sram_slow_on (ramp + connect)");
  board.sram_slow_on();   // slow ramp + Vcca connect
  Serial.println("manually connect aport pwr (tmp)");   // manual aport pwr connect
  delay(1500);

  // TODO: physically add tps22917 switch to board

  // simple test read routine, read a page (32 bytes)
  uint16_t addr = 0;
  uint8_t buf[32] = {'\0'};

  Serial.print(addr, HEX);
  Serial.print(": ");
  xsram.Spi23K640Read32(addr, SRAM_CS_PIN, buf, sizeof(buf));   // test read
  for (uint8_t i=0; i<32; i++)
  {
    uint8_print_hex(buf[i], true);
  }
  board.sram_pwr_off();   // handles Vcc disconnect and SRAM power off sequencing (needs tps22917)
  Serial.println("\nmanually disconnect aport pwr");    // temporary measure until tps22917 is added
  delay(1500);
  Serial.println("\nslow ramp test complete");
  delay(500);

//  myFile = SD.open(filename, FILE_WRITE);
//  if (myFile)
//  {
//    // TODO: write test data to sd or save on host
//    myFile.close();
//  }
//  else
//  {
//    Serial.println("error opening file");
//  }

  ////////////////////////////
  // fast ramp test routine //
  ////////////////////////////

  Serial.println("execute sram_fast_on (fast ramp + connect)");
  delay(500);
  board.sram_fast_on();   // fast ramp + Vcca connect
  Serial.println("manually connect aport pwr (tmp)");   // manual aport pwr connect
  delay(1500);    // temporary delay for manual connect
  //delay(500);                   // settle time prior to read

  // 23K640 maxram is 8192, maxpage is 256 (datasheet erroneously states 1024 pages) //

  addr = 0;
  memset(buf, '\0', sizeof(buf));
  Serial.print(addr, HEX);
  Serial.print(": ");
  xsram.Spi23K640Read32(addr, SRAM_CS_PIN, buf, sizeof(buf));   // test read
  for (uint8_t i=0; i<32; i++)
  {
    uint8_print_hex(buf[i], true);
  }
  board.sram_pwr_off();   // handles Vcc disconnect and SRAM power off sequencing (needs tps22917)
  Serial.println("\nmanually disconnect aport pwr");    // temporary measure until tps22917 is added
  delay(1500);
  Serial.println("\nfast ramp test complete");
  delay(1000);
  SPI.end();
  Serial.println("done");
}

void loop()
{
  Serial.print(".");
  delay(1000);
}
