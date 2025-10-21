// XSRAM.cpp - the derived class 'XSRAM' inherits from
// the base SRAM class written by Ade Setyawan.  Adds functionality
// for alternate bit selection method, while preserving original code.
//
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

#include "XSRAM.h"

#define SRAM_23K640         // using 23K640 for ramp method

// 23K640 instruction set/format (READ, WRITE, RDSR, WRSR) is the same as 23LC1024
#define READ 0x03   // spi read cmd
#define WRITE 0x02  // spi write cmd
#define RDSR 0x05   // read status register
#define WRSR 0x01   // write status register
#define MAX_CLK 20000000   // MAX SPI transfer rate (Hz)

XSRAM::XSRAM() : SRAM(){
#ifdef SRAM_23K640    // re-assign vars from 23LC1024 (SRAM class defaults)
  maxram = 8192;      // bytes in 23K640 SRAM
  maxpage = 256;      // pages in 23K640 SRAM - datasheet section 2.2, "1024 pages" is incorrect
#endif
}

// hide, since using DAC instead
void XSRAM::set_pin_power(){}
void XSRAM::turn_off(){}
void XSRAM::turn_on(){}

uint8_t XSRAM::get_pin_cs() const{
  return pin_cs;
}

uint8_t XSRAM::get_pin_hold() const{
  return pin_hold;
}

uint8_t XSRAM::get_pin_power() const{
  return pin_power;
}

uint8_t XSRAM::get_pin_mosi() const{
  return pin_mosi;
}

uint8_t XSRAM::get_pin_miso() const{
  return pin_miso;
}

uint8_t XSRAM::get_pin_sck() const{
  return pin_sck;
}

// read byte
uint8_t XSRAM::Spi23K640Read8(uint16_t addr, uint8_t cs_pin)
{
  uint8_t data;

  SPI.beginTransaction(SPISettings(MAX_CLK, MSBFIRST, SPI_MODE0));    // 23K640, 20MHz max clock
  digitalWrite(cs_pin, LOW);
  SPI.transfer(READ);
  SPI.transfer(addr >> 8);          // high byte
  SPI.transfer(addr & 0xFF);        // low byte
  data = SPI.transfer(0x00);        // write dummy byte to read
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();
  return data;
}

// write byte
void XSRAM::Spi23K640Write8(uint16_t addr, uint8_t cs_pin, uint8_t data)
{
  SPI.beginTransaction(SPISettings(MAX_CLK, MSBFIRST, SPI_MODE0));
  digitalWrite(cs_pin, LOW);
  SPI.transfer(WRITE);
  SPI.transfer(addr >> 8);      // high byte
  SPI.transfer(addr & 0xFF);    // low byte
  SPI.transfer(data);           // write byte
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();
}

// read page
int XSRAM::Spi23K640Read32(uint16_t addr, uint8_t cs_pin, uint8_t *buf, size_t bufsize)
{
  if (bufsize < pagesize)   // sanity check
  {
    return -1;    // insufficient buffer
  }
  SPI.beginTransaction(SPISettings(MAX_CLK, MSBFIRST, SPI_MODE0));
  digitalWrite(cs_pin, LOW);
  SPI.transfer(READ);
  SPI.transfer(addr >> 8);
  SPI.transfer(addr & 0xFF);

  for (uint8_t i=0; i<32; i++)
  {
    buf[i] = SPI.transfer(0x00);   // 32 dummy writes --> 32 read bytes
  }
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();
  return 0;
}

// write page
int XSRAM::Spi23K640Write32(uint16_t addr, uint8_t cs_pin, uint8_t *buf, size_t bufsize)
{
  if (bufsize < pagesize)   // sanity check
  {
    return -1;    // buffer too small
  }
  SPI.beginTransaction(SPISettings(MAX_CLK, MSBFIRST, SPI_MODE0));
  digitalWrite(cs_pin, LOW);
  SPI.transfer(WRITE);
  SPI.transfer(addr >> 8);
  SPI.transfer(addr & 0xFF);

  for (uint8_t i=0; i<32; i++)
  {
    SPI.transfer(buf[i]);
  }
  digitalWrite(cs_pin, HIGH);
  SPI.endTransaction();
  return 0;
}

// read all and print as hex (one page at a time)
void XSRAM::Spi23K640ReadAllPrintHex(bool sep)
{
  uint8_t buf[32];

  char hexval[3] = {'\0'};
  const char table[] = "0123456789ABCDEF";

  for (uint32_t i=0; i<maxpage; i++)
  {
    Spi23K640Read32(i*32, pin_cs, buf, sizeof(buf));
    for (uint8_t n=0; n<32; n++)
    {
      hexval[0] = table[buf[n] >> 4];
      hexval[1] = table[buf[n] & 0xF];
      Serial.print(hexval);
      if (sep)
      {
        Serial.print(" ");
      }
    }
  }
}

void XSRAM::Spi23K640WriteAllZero()
{
  uint8_t buf[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  for (uint32_t i=0; i<maxpage; i++)
  {
    Spi23K640Write32(i*32, pin_cs, buf, sizeof(buf));
  }
}

void XSRAM::Spi23K640WriteAllOne()
{
	uint8_t buf[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  };

  for (uint32_t i=0; i<maxpage; i++)
  {
    Spi23K640Write32(i*32, pin_cs, buf, sizeof(buf));
  }
}