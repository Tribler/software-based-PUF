// ------------------------------------------------------------------------
//
//        File: SRAM_CY62256N.h
//        Date: April 15, 2018
//        Created by: Ade Setyawan Sajim as part of his MSc Thesis project
//
// This code can be use to read and write to SRAM Cypress CY62256NLL using
// Arduino Mega 2560. Adapted from work by johnzl-777 which can be retrieved at
// https://github.com/johnzl-777/SRAM-Read-Write
//

#ifndef SRAM_CY62256N_H
#define SRAM_CY62256N_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "string.h"
#include "Arduino.h"
#include <SPI.h>

using namespace std;

class SRAM_CY62256N {
private:
  uint32_t maxram = 32768;

  void set_addr (uint16_t address);

  uint8_t data_op (char rw, uint8_t data);
public:
  SRAM_CY62256N();

  uint32_t get_max_ram();

  uint8_t read(uint16_t address);

  void write(uint16_t address, uint8_t data_byte);

};

#endif //SRAM_H
