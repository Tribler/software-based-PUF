//
// Created by Ade Setyawan on 01/17/18.
//

#ifndef SRAM_H
#define SRAM_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "string.h"
#include "Arduino.h"
#include <SPI.h>

using namespace std;

class SRAM {
private:
  uint32_t maxram = 131072; //number of bytes in 23LC1024 SRAM
  uint32_t maxpage = 4096;  //number of pages in 23LC1024 SRAM
  uint8_t pin_cs;
  uint8_t pin_hold;
  uint8_t pin_power;
  uint8_t pin_mosi;
  uint8_t pin_miso;
  uint8_t pin_sck;

  uint8_t Spi23LC1024Read8(uint32_t address, uint8_t cs_pin);
  void Spi23LC1024Write8(uint32_t address, uint8_t data_byte, uint8_t cs_pin);
  void Spi23LC1024Read32(uint32_t address, uint8_t cs_pin, uint8_t* buff);
  void Spi23LC1024Write32(uint32_t address, uint8_t* buffer170, uint8_t cs_pin);

public:
  SRAM();

  uint32_t get_max_ram();

  uint32_t get_max_page();

  void set_pin_cs(uint8_t pin);

  void set_pin_hold(uint8_t pin);

  void set_pin_power(uint8_t pin);

  void set_pin_mosi(uint8_t pin);

  void set_pin_miso(uint8_t pin);

  void set_pin_sck(uint8_t pin);

  void turn_off();

  void turn_on();

  void read_all();

  void read_page(uint32_t page, uint8_t* result);

  uint8_t read_single(long location);

  void write_all_zero();

  void write_all_one();

  void write_single(long address, uint8_t data);

  void write_page(long page, bool is_one);
};

#endif //SRAM_H
