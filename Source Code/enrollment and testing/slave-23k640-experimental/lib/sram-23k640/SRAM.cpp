// This file is part of software-based-PUF,
// https://github.com/Tribler/software-based-PUF
// Modifications and additions Copyright (C) 2024 myndcryme.
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

#include "SRAM.h"

#define RDSR        5
#define WRSR        1
#define READ        3
#define WRITE       2

SRAM::SRAM(){

}

uint32_t
SRAM::get_max_ram(){
  return maxram;
}

uint32_t
SRAM::get_max_page(){
  return maxpage;
}

void
SRAM::set_pin_cs(uint8_t pin){
  pin_cs = pin;
}

void
SRAM::set_pin_hold(uint8_t pin){
  pin_hold = pin;
}

void
SRAM::set_pin_power(uint8_t pin){
  pin_power = pin;
}

void
SRAM::set_pin_mosi(uint8_t pin){
  pin_mosi = pin;
}

void
SRAM::set_pin_miso(uint8_t pin){
  pin_miso = pin;
}

void
SRAM::set_pin_sck(uint8_t pin){
  pin_sck = pin;
}


uint8_t
SRAM::Spi23LC1024Read8(uint32_t address, uint8_t cs_pin)
{
  uint8_t read_byte;

  digitalWrite(cs_pin, LOW);
  SPI.transfer(READ);
  SPI.transfer((uint8_t)(address >> 16));
  SPI.transfer((uint8_t)(address >> 8) & 0xFF);
  SPI.transfer((uint8_t)address);
  read_byte = SPI.transfer(0x00);
  digitalWrite(cs_pin, HIGH);

  return read_byte;
}

void
SRAM::Spi23LC1024Write8(uint32_t address, uint8_t data_byte, uint8_t cs_pin)
{
  digitalWrite(cs_pin, LOW);
  SPI.transfer(WRITE);
  SPI.transfer((uint8_t)(address >> 16) & 0xff);
  SPI.transfer((uint8_t)(address >> 8) & 0xff);
  SPI.transfer((uint8_t)address);
  SPI.transfer(data_byte);
  digitalWrite(cs_pin, HIGH);
}

void
SRAM::Spi23LC1024Read32(uint32_t address, uint8_t cs_pin, uint8_t* buff)
{
  // uint32_t i;                // larger than necessary
  // uint8_t read_page;         // unnecessary

  digitalWrite(cs_pin, LOW);
  SPI.transfer(READ);
  SPI.transfer((uint8_t)(address >> 16));
  SPI.transfer((uint8_t)(address >> 8));
  SPI.transfer((uint8_t)address);

  for (int i=0; i<32; i++)
  {
    buff[i] = SPI.transfer(0x00);
  }
  digitalWrite(cs_pin, HIGH);

//  for (i = 0; i < 32; i++)
//  {
//    read_page = SPI.transfer(0x00);
//    buff[i] = read_page;
//  }
//  digitalWrite(cs_pin, HIGH);
}

void
SRAM::Spi23LC1024Write32(uint32_t address, uint8_t* buffer170, uint8_t cs_pin)
{
  // uint32_t i;    // larger than necessary

  digitalWrite(cs_pin, LOW);
  SPI.transfer(WRITE);
  SPI.transfer((uint8_t)(address >> 16));
  SPI.transfer((uint8_t)(address >> 8));
  SPI.transfer((uint8_t)address);
  for (int i=0; i<32; i++)
  {
    SPI.transfer(buffer170[i]);
  }
  digitalWrite(cs_pin, HIGH);
}

void
SRAM::turn_off(){
  SPI.end();

  pinMode(pin_miso, OUTPUT);
  pinMode(pin_mosi, OUTPUT);
  pinMode(pin_sck, OUTPUT);

  digitalWrite(pin_miso, LOW);
  digitalWrite(pin_mosi, LOW);
  digitalWrite(pin_sck, LOW);
  digitalWrite(pin_power, LOW);
  digitalWrite(pin_hold, LOW);
  digitalWrite(pin_cs, HIGH);   // corrected
  // digitalWrite(pin_cs, LOW);
}

void
SRAM::turn_on(){
  pinMode(pin_cs, OUTPUT);
  pinMode(pin_hold, OUTPUT);
  pinMode(pin_power, OUTPUT);

  digitalWrite(pin_cs, LOW);
  digitalWrite(pin_hold, HIGH);
  digitalWrite(pin_power, HIGH);

  SPI.begin();
}

void
SRAM::read_all(){
  uint8_t buff[32];

  for (int i = 0; i < get_max_page(); i++)
  {
    Spi23LC1024Read32(i, pin_cs, buff);
    for (int n = 0; n < 32; n++)
    {
      // Serial.println(buff[n], HEX);
      // print_binary(buff[n], 8);
    }
  }
}

void
SRAM::read_page(uint32_t page, uint8_t* result){
  Spi23LC1024Read32(page, pin_cs, result);
}

uint8_t
SRAM::read_single(long location){
  return Spi23LC1024Read8(location, pin_cs);
}

void
SRAM::write_all_zero(){
  uint8_t buffer00[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  for (int i = 0; i < maxpage; i++)
  {
    Spi23LC1024Write32(i, buffer00, pin_cs);
  }
}

void
SRAM::write_all_one(){
  uint8_t bufferFF[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  };

  for (int i = 0; i < maxpage; i++)
  {
    Spi23LC1024Write32(i, bufferFF, pin_cs);
  }
}

void
SRAM::write_page(long page, bool is_one){
  uint8_t bufferFF[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  };
  uint8_t buffer00[32] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  Spi23LC1024Write32(page, is_one ? bufferFF : buffer00, pin_cs);
}

void
SRAM::write_single(long address, uint8_t data){
  Spi23LC1024Write8(address, data, pin_cs);
}
