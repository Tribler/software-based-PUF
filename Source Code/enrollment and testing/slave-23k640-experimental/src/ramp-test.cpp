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

#define DAC_ADDR 0x62   // ADAFRUIT_MCP4725_I2C_ADDR
#define DN_RES 4096     // DAC input code resolution (12 bits)
#define DN_VDD 4095     // Dn (input code) == supply voltage, VDD
#define VDD 3.300       // DAC supply voltage
#define VSS 0.0         // ground reference

#define HOLD_PIN 11
#define MISO_PIN 50
#define MOSI_PIN 51
#define SCK_PIN 52
#define SD_CS_PIN 7       // chip select pin for SD card breakout
#define SRAM_CS_PIN 10    // chip select pin for SRAM IC
#define OE_PIN 3          // OE (74LVC1G125 output enable)
#define IN1_PIN 8         // IN1 (LMS4684 input1)
#define OE_TXB_PIN 5      // output enable for TXB level shifter

PUFBoard board;         // PUF breakout board instance

XSRAM& xsram = board.getXSRAM();    // allow access XSRAM instance directly

const char HEX_TABLE[] = "0123456789ABCDEF";
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

void setup()
{
  // set pin assignments
  board.set_pin_in1(IN1_PIN);   // puf board IC LMS4684 in1 pin
  board.set_pin_oe(OE_PIN);     // puf board IC 74LVC1G125 oe pin
  board.set_pin_oe_txb(OE_TXB_PIN);
  //board.set_pin_vcc(VCC_PIN);

  // sram is now mounted on board but pin assignments are still set using original class
  // setters (actually extended class setters, but they are inherited and equivalent)
  xsram.set_pin_cs(SRAM_CS_PIN);
  xsram.set_pin_hold(HOLD_PIN);
  xsram.set_pin_mosi(MOSI_PIN);
  xsram.set_pin_miso(MISO_PIN);
  xsram.set_pin_sck(SCK_PIN);

  // then initialize the board
  board.init_board();
  delay(500);

  File myFile;              // SD card file
  bool b = false;
  const char *filename = "test.txt";
  float vdd_min = 0.58;    // ramp begin voltage
  float vdd_spl = VDD;     // ramp end voltage

  /* Dn = input code (0 to 4095)
   * Vout = VDD*(Dn/4096)
   * Dn = (Vout*4096)/VDD
   */

  uint32_t dn;          // input code
  uint32_t dn_min = floor((vdd_min*DN_RES)/vdd_spl);    // Dn (input code) == vdd_min voltage
  uint32_t steps = DN_VDD - dn_min;          // available DAC steps between min and max voltages
  uint32_t hold_ms = 1000;        // hold time in ms
  uint32_t ramp_ms = 1000;        // ramp time in ms

  // TODO:  account for loop and operations time for an accurate step value
  uint32_t step_usec = floor((ramp_ms*1000)/steps);

  Serial.begin(115200);
  delay(500);

  Serial.println("\ninitializing DAC");
  board.dac_begin(DAC_ADDR);    // initialize DAC and join I2C bus
  delay(500);
  Serial.println("enable voltage level shifter");
  board.txb_enable();   // on for read operation

//  Serial.println("sram_power_on\npin_in1 HIGH, pin_oe HIGH");
//  board.sram_power_on();
//  delay(1000);
//  Serial.println("sram_power_off\npin_in1 LOW, pin_oe HIGH");
//  board.sram_power_off();
//  delay(500);

//  Serial.print("init sd card...");
//  if (!SD.begin(SD_CS_PIN))
//  {
//    Serial.println("sd init failed");
//    delay(1000);
//    while(1);    // wait forever till sd bring up
//  }
//  Serial.println("done");

  delay(5000);

  // slow ramp test routine
  Serial.println("config slow ramp");
  board.config_slow_ramp();               // configure pins before DAC output
  delay(1000);

  Serial.println("\nbegin slow ramp test");
  board.dac_set_voltage(dn_min, false);   // set initial voltage with vdd_min input code
  delay(hold_ms);                         // hold min voltage for specified ms
  for (dn=dn_min; dn<DN_VDD; dn++)        // voltage ramp loop
  {
    board.dac_set_voltage(dn, false);
    delayMicroseconds(133);               // results in approximately a one sec ramp time
    // delayMicroseconds(step_usec);      // TODO: step_usec needs a correction factor to work properly
  }

  // slow ramp complete, must pull HOLD up to VCC
  digitalWrite(HOLD_PIN, HIGH);   // TODO: this should be handled elsewhere, in a callable slow ramp function

  // read sram test
  Serial.println("read page test results (slow ramp)");
  delay(100);

  // TODO: check xsram.get_max_page(), 23LC1024 4096 total pages, 23K640 1024 pages
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
  board.sram_power_off();
  Serial.println("slow ramp test complete");
  delay(5000);

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

  // fast ramp quick test routine
  Serial.println("\nbegin fast ramp test");
  board.config_fast_ramp();
  delay(1000);
  Serial.println("sram_fast_on");
  board.sram_fast_on();
  delay(1000);
  Serial.println("read page test results (fast ramp)");
  delay(100);

  // 23K640 maxram is 32768, maxpage is
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
  Serial.println("sram_power_off");
  board.sram_power_off();
  delay(500);
  Serial.println("disable voltage level translator (TXB)");
  board.txb_disable();
  delay(1000);
}

void loop()
{
  Serial.print(".");
  delay(1000);
}
