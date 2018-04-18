// ------------------------------------------------------------------------
//
//        File: SRAM_CY62256N.cpp
//        Date: April 15, 2018
//        Created by: Ade Setyawan Sajim as part of his MSc Thesis project
//
// This code can be use to read and write to SRAM Cypress CY62256NLL using
// Arduino Mega 2560. Adapted from work by johnzl-777 which can be retrieved at 
// https://github.com/johnzl-777/SRAM-Read-Write

#include "SRAM_CY62256N.h"

#define OE_LOW()    digitalWrite(13, 0)   //Sets OE LOW
#define CE_LOW()    digitalWrite(12, 0)   //Sets CE LOW
#define WE_LOW()    digitalWrite(11, 0)   //Sets WE LOW
#define WE_HIGH()   digitalWrite(11, 1)   //Sets WE HIGH

SRAM_CY62256N::SRAM_CY62256N(){

}

uint32_t
SRAM_CY62256N::get_max_ram(){
  return maxram;
}

/*
 * Set Address Function, Outputs the address through two pin registers
 * Arguments: Address to be outputted
 */
void
SRAM_CY62256N::set_addr (uint16_t address)
{

  PORTA = address & 0xff;         //Takes the first half of the address and sets it to PORTA (Lower Address Byte)
  PORTC = (address >> 8) & 0xff;  //Takes the second half of the address and sets it to PORTC (Upper Address Byte)

}

/*
 * Data Operation Function, controls the data IO from the Arduino
 * Arguments: Read/Write option and data to be written
 */

uint8_t
SRAM_CY62256N::data_op (char rw, uint8_t data)
{
  if(rw == 'w')
  {
    /*
     * If RW option is set to char 'w' for WRITE
     */
    DDRL = 0xff;   //Set Data Direction on L register pins to OUTPUT
    PORTL = data;  //Output Data through L pin data register
    return PORTL;  //Return the data outputted as confirmation
  }
  else if (rw == 'r')
  {
   /*
    * If RW option is set to char 'r' for READ, read data
    */
    PORTL = 0x00;  //Erase any data still being held from previous write operation in L pin data register
    DDRL = 0x00;   //Set L register pins to INPUT
    return PINL;   //Return values read in the pin register
  }

}

void
SRAM_CY62256N::write(uint16_t address, uint8_t data)
{
  /*
   * Write operation uses a WE (Write Enabled Controlled) Write Cycle.
   */
  OE_LOW();             //OE is continuously LOW
  CE_LOW();             //CE is continuously LOW
  WE_HIGH();            //WE starts off HIGH
  delay(1);             //Delay just to ensure signals stay HIGH/LOW long enough

  set_addr(address);    //Address applied first
  WE_LOW();             //WE goes from HIGH to LOW
  data_op('w', data);   //Data applied to data bus
  WE_HIGH();            //WE goes from LOW to HIGH

}

/*
 * Read Data Function
 * Arguments: Address to be used to obtain data
 */

uint8_t
SRAM_CY62256N::read(uint16_t address)
{

  /*
   * Read Operation uses "Read Cycle 1", see PDF documentation linked
   */
  WE_HIGH();     //WE set to HIGH at all times for Type 1 Read
  OE_LOW();      //OE set to LOW at all times for Type 1 Read
  CE_LOW();      //CE set to LOW at all times for Type 1 Read

  set_addr(address);        //Address applied first
  return (data_op('r', NULL));  //Read data operation is committed, data obtained is returned

}
