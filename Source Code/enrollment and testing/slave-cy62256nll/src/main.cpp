#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include <stdio.h>
#include <Tools.h>
#include <BCH.h>
#include <SRAM_CY62256N.h>
#include <SPI.h>
#include <SD.h>

#define TURN_OFF_CMD 40
#define TURN_ON_CMD 41
#define READ_PAGE_CMD 42
#define READ_SINGLE_CMD 43
#define WRITE_PAGE 47
#define WRITE_SINGLE 48
#define READ_BIT_CMD 49

#define NEW_CHALLENGE_FOR_MICRO_SD 50
#define APPEND_CHALLENGE_TO_MICRO_SD 51
#define FINISH_WRITING_CHALLENGE_TO_MICRO_SD 52
#define GENERATE_HELPER_DATA 53
#define GET_KEYS 54
#define GET_UID 55          // added cmd
#define UID_BUF_SIZE 36     // uid str buffer

#define PIN_POWER_ANALOG A8
#define PIN_POWER 9

SRAM_CY62256N sram;
char command[6];
uint8_t result;
uint16_t address;
int count = 0;

uint8_t key_32[32];
uint8_t key_256[32];

uint8_t puf_binary_new[8*37];
uint8_t helper_data_new[7*37];
uint8_t key_per_row[37];

BCH bch;
Tools tools;

File myFile;

void turn_on_sram(){
  analogWrite(PIN_POWER, 255);
}

void turn_off_sram(){
  analogWrite(PIN_POWER, 0);

  for (int i=11; i<=13; i++){
    digitalWrite(i, LOW);
  }
  for (int i=22; i<=29; i++){
   digitalWrite(i, LOW);
  }
  for (int i=31; i<=37; i++){
    digitalWrite(i, LOW);
  }
  for (int i=42; i<=49; i++){
    digitalWrite(i, LOW);
  }
}

boolean readBit(long location){
  uint8_t result = sram.read(floor(location / 8));
  return result >> (7 - (location % 8)) & 0x1 == 1;
}

void set(){
  sram = SRAM_CY62256N();

  DDRB = 0b11100000; //Pin 13, 12, 11 set to output
  DDRA = 0b11111111; //Pins 22-29 Set as Output (Lower Byte of Address)
  DDRC = 0b01111111; //Pins 37-31 Set as Output (Upper Byte of Address)

  pinMode(9, OUTPUT);

  Serial.begin(115200);
  while (!Serial)
  {
    ;       // proper wait for serial connection
  }
  delay(1000);

  bch = BCH();
  tools = Tools();
  bch.initialize();               /* Read m */

}

void decode(){
  uint8_t helper_data_padded_new[8*37];
  uint8_t xor_reproduction_new[8*37];
  uint8_t reconstructed_key_new[37];

  memset(helper_data_padded_new, 0, sizeof(helper_data_padded_new));
  memset(xor_reproduction_new, 0, sizeof(xor_reproduction_new));
  memset(reconstructed_key_new, 0, sizeof(reconstructed_key_new));

  int row = 37;
  int n = bch.get_n();
  int k = bch.get_key_length();

  int k_length_bit = row;//tools.ceil(k*row,8);
  int n_length_bit = row * 8;//tools.ceil(n*row,8);

  /************************************
   ****** REPRODUCTION PROCEDURE ******
   ************************************/
  /******************** ASSERT HELPER DATA PADDED**********************/
  for (int i = 0; i < row; i++) {
      // memcpy(&helper_data_padded_new[i * 8], &helper_data[i * 7], 7 * sizeof(uint8_t));
      memcpy(&helper_data_padded_new[i * 8], &helper_data_new[i * 7], 7 * sizeof(uint8_t));
  }

  /******************** ASSERT XOR REPRODUCTION **********************/
  for (int i = 0; i < row; i++) {
     for (int j = 0; j < 8; j++) {
       xor_reproduction_new[i*8 + j] = puf_binary_new[i*8 + j] ^ helper_data_padded_new[i*8 + j];
       // xor_reproduction_new[i*8 + j] = puf_binary_default[i*8 + j] ^ helper_data_padded_new[i*8 + j];
     }
  }

  /******************** ASSERT RECONSTRUCTED KEY **********************/
  for (int i = 0; i < row; i++) {
      bch.decode_bch(&xor_reproduction_new[i * 8], &reconstructed_key_new[i]);
  }

  tools.convert_key_back(reconstructed_key_new, key_32);
}

void initializeSD(){
  if (!SD.begin(7)) {
    delay(2000);
  }
}

boolean isValidNumber(String str){
   if (str.length() == 0)
    return false;
   for(byte i=0;i<str.length();i++)
   {
     if(!isDigit(str.charAt(i)))
      return false;
   }
   return true;
}

String convertString(String a){
  String s;
  boolean isNum;
  for(byte i=0;i<a.length();i++)
  {
      isNum = isDigit(a.charAt(i));
      if(isNum)
        s += a.charAt(i);
  }
  return s;
}

void readSRAMfromMicroSD(){
  memset(puf_binary_new, 0, sizeof(puf_binary_new));
  uint8_t result;

  // read
  int index = 0, i=0;

  String name = "c.txt";
  String a, b;
  long thisItem;
  myFile = SD.open(name);
  if (myFile) {
    while (myFile.available()) {
      a = myFile.readStringUntil('\n');
      b = convertString(a);
      if (isValidNumber(b)){
        thisItem = b.toInt();
        if (readBit(thisItem))
          puf_binary_new[index] = puf_binary_new[index] | 0x1;
        if ((i+1)%8==0){
          index++;
        } else{
          puf_binary_new[index] = puf_binary_new[index] << 1;
        }
        i++;
      }
    }
    myFile.close();
  }
}

void readHelperDatafromMicroSD(){
  memset(helper_data_new, 0, sizeof(helper_data_new));
  uint8_t result;

  // read
  int index = 0, i=0;

  String name = "h.txt";
  String a, b;
  long thisItem;
  myFile = SD.open(name);
  if (myFile) {
    while (myFile.available()) {
      a = myFile.readStringUntil('\n');
      b = convertString(a);
      if (isValidNumber(b)){
        thisItem = b.toInt();
        helper_data_new[i] = thisItem;
        i++;
      }
    }
    myFile.close();
  }
}

void gen_key256(){
  turn_on_sram();
  readSRAMfromMicroSD();
  turn_off_sram();

  memset(key_256, 0, 32 * sizeof(uint8_t));
  memcpy(&key_256, &puf_binary_new, 32 * sizeof(uint8_t));
}

void gen_helper_data(){
  gen_key256();

  tools.gen_key_per_row(key_256, key_per_row);

  uint8_t encoded_new[8*37];
  uint8_t xor_enroll_new[8*37];

  int row = 37;
  int n = bch.get_n();
  int k = bch.get_key_length();
  int index, shift, ii;

  /******************** ASSERT ENCODED **********************/
  for (int i = 0; i < row; i++) {
      bch.encode_bch(&key_per_row[i], &encoded_new[i * 8]);
  }

  /******************** ASSERT XOR ENROLL **********************/
  for (int i = 0; i < row; i++) {
      for (int j = 0; j < 8; j++) {
        xor_enroll_new[i*8 + j] = encoded_new[i*8 + j] ^ puf_binary_new[i*8 + j];
      }
  }

  /******************** ASSERT HELPER DATA **********************/
  for (int i = 0; i < 37; i++) {
      memcpy(&helper_data_new[i * 7], &xor_enroll_new[i * 8], 7 * sizeof(uint8_t));
  }

  File myFile = SD.open("h.txt", O_WRITE | O_CREAT | O_TRUNC);
  if (myFile) {
    for (int i = 0; i < 37*7;i++){
      myFile.println(helper_data_new[i]);
    }
    myFile.close();
  }
}

void check_command()
{
  int q;
  long a;

  if (command[0] != 99)
    return;
  switch (command[1])
  {
    case TURN_OFF_CMD:
      turn_off_sram();
      if (command[2] > 0)
      {
        delay(1000);
      }
      q = analogRead(PIN_POWER_ANALOG);

      Serial.write(99);
      Serial.write(TURN_OFF_CMD);
      Serial.write((q >> 8) & 0xFF);
      Serial.write(q & 0xFF);
      for (int i=0; i < 36-4; i++)
        Serial.write(0);
      break;

    case TURN_ON_CMD:
      turn_on_sram();
      if (command[2] > 0)
      {
        delay(1000);
      }
      q = analogRead(PIN_POWER_ANALOG);

      Serial.write(99);
      Serial.write(TURN_ON_CMD);
      Serial.write((q >> 8) & 0xFF);
      Serial.write(q & 0xFF);
      for (int i=0; i < 32; i++)
        Serial.write(0);
      break;

    case READ_PAGE_CMD:
      address = 0;
      memcpy(&address, &command[2], 2);

      Serial.write(99);
      Serial.write(READ_PAGE_CMD);
      Serial.write((address >> 8) & 0xFF);
      Serial.write(address & 0xFF);

      address = address * 32;
      for (int i=0; i < 32; i++)
      {
        result = sram.read(address + i);
        Serial.write(result);
      }
      break;

    case READ_SINGLE_CMD:
      address = 0;
      memcpy(&address, &command[2], 2);

      Serial.write(99);
      Serial.write(READ_SINGLE_CMD);
      Serial.write((address >> 8) & 0xFF);
      Serial.write(address & 0xFF);

      result = sram.read(address);
      Serial.write(result);

      for (int i=0; i < 36-5; i++)
        Serial.write(0);
      break;

    case READ_BIT_CMD:
      boolean c;
      memcpy(&a, &command[2], 4);

      Serial.write(99);
      Serial.write(READ_BIT_CMD);
      Serial.write((uint8_t) (a >> 24) & 0xFF);
      Serial.write((uint8_t) (a >> 16) & 0xFF);
      Serial.write((uint8_t) (a >> 8) & 0xFF);
      Serial.write(a & 0xFF);

      c = readBit(a);
      Serial.write(c ? 1 : 0);

      for (int i=0; i < 36-7; i++)
        Serial.write(0);
      break;

    case WRITE_SINGLE:
      address = 0;
      memcpy(&address, &command[2], 2);
      result = command[4];
      sram.write(address, result);
      Serial.write(99);
      Serial.write(WRITE_SINGLE);
      Serial.write((address >> 8) & 0xFF);
      Serial.write(address & 0xFF);
      for (int i=0; i < 36-4; i++)
        Serial.write(0);
      break;

    case WRITE_PAGE:
      address = 0;
      memcpy(&address, &command[2], 2);

      Serial.write(99);
      Serial.write(WRITE_PAGE);
      Serial.write((address >> 8) & 0xFF);
      Serial.write(address & 0xFF);

      address = address * 32;
      if (command[4] == 1)
      {
        for (int i=0; i < 32; i++)
        {
          sram.write(address + i, 0xFF);
          Serial.write(sram.read(address + i));
        }
      }
      else
      {
        for (int i=0; i < 32; i++)
        {
          sram.write(address + i, 0x00);
          Serial.write(sram.read(address + i));
        }
      }
      break;

    case APPEND_CHALLENGE_TO_MICRO_SD:
      memcpy(&a, &command[2], 4);
      myFile.println(a);

      Serial.write(99);
      Serial.write(APPEND_CHALLENGE_TO_MICRO_SD);
      Serial.write((uint8_t) (a >> 24) & 0xFF);
      Serial.write((uint8_t) (a >> 16) & 0xFF);
      Serial.write((uint8_t) (a >> 8) & 0xFF);
      Serial.write((uint8_t) a & 0xFF);
      for (int i=0; i < 36-6; i++)
        Serial.write(0);
      break;

    case NEW_CHALLENGE_FOR_MICRO_SD:
      myFile = SD.open("c.txt", O_WRITE | O_CREAT | O_TRUNC);

      Serial.write(99);
      Serial.write(NEW_CHALLENGE_FOR_MICRO_SD);
      Serial.write(myFile ? 1 : 0);
      for (int i=0; i < 33; i++)
        Serial.write(0);
      break;

    case FINISH_WRITING_CHALLENGE_TO_MICRO_SD:
      myFile.close();

      Serial.write(99);
      Serial.write(FINISH_WRITING_CHALLENGE_TO_MICRO_SD);
      for (int i=0; i < 36-2; i++)
        Serial.write(0);
      break;

    case GENERATE_HELPER_DATA:
      gen_helper_data();
      Serial.write(99);
      Serial.write(GENERATE_HELPER_DATA);
      for (int i=0; i < 36-2; i++)
        Serial.write(0);
      break;

    case GET_KEYS:
      // gen_key256();
      readSRAMfromMicroSD();
      readHelperDatafromMicroSD();
      decode();
      Serial.write(99);
      Serial.write(GET_KEYS);
      Serial.write(0);
      Serial.write(0);
      for (int i=0; i < 32; i++)
        Serial.write(key_32[i]);
      break;

    case GET_UID:       // get arduino uid (uses ArduinoUniqueID)
      size_t i=0;
      char str[UID_BUF_SIZE] = {'\0'};      // char array since String class can cause issues

      for (i=0; i<UniqueIDsize; i++)        // size_t i
      {
        if (UniqueID[i] < 0x10)     // single character (i.e. decimal value 10 is hex value A)
          sprintf(str + (i * 2), "0%X", UniqueID[i]);   // shift array start position, pad single char (example: 0A)
        else
          sprintf(str + (i * 2), "%X", UniqueID[i]);    // shift array start position
      }
      // shouldn't be necessary but ensure newline and terminate
      str[UID_BUF_SIZE-2] = '\n';       // end of data marker
      str[UID_BUF_SIZE-1] = '\0';
      Serial.write(str);    // writes uid as binary data
      break;

    default:
      Serial.write(99);
      for (int i=0; i < 36-1; i++)
        Serial.write(0);
      break;
  }
  memset(command, 0, 6);
}

void setup()
{
  set();
  initializeSD();
  pinMode(PIN_POWER, OUTPUT);
}

void loop()
{
  if (Serial.available() > 0)
  {
    command[count] = Serial.read();
    count++;
    if (count == 6)
    {
      check_command();
      count = 0;
    }
  }
}
