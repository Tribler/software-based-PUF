#include <Arduino.h>
#include <stdio.h>
#include <Tools.h>
#include <BCH.h>
#include <SRAM_CY62256N.h>
#define SERIAL_RX_BUFFER_SIZE 128       // override HardwareSerial.h (included in SPI.h)
#define SERIAL_TX_BUFFER_SIZE 128       // override HardwareSerial.h (default is 64)
#include <SPI.h>
#include <SD.h>
#include <Crypto.h>
#include <SHA3.h>
#include <string.h>
#include <AES.h>
#include <CTR.h>

#define PIN_POWER_ANALOG A8
#define PIN_POWER 9
#define BUF_SIZE 5          // first byte is prefix code
#define MAX_COUNT 120
#define PIN_SAMPLE_SIZE 16
#define MSGBUF_SIZE 128                 // serial_print() message buffer size
#define SEED_ARDUINO_CMD 0x39           // flag preceding incoming seed data
#define DEBUG

SRAM_CY62256N sram;

char msgbuf[MSGBUF_SIZE] = {'\0'};      // global message buffer
uint8_t helper_data_new[7 * 37];
uint8_t puf_binary_new[8 * 37];

boolean readBit(long location) {
  uint8_t result = sram.read(floor(location / 8));
  return result >> (7 - (location % 8)) & 0x1 == 1;
}

// format message string then serial print and flush (avoids display issues)
void serial_print(char *str)
{
  snprintf(msgbuf, MSGBUF_SIZE, str);       // truncates if str > msgbuf
  Serial.print(msgbuf);
  Serial.flush();
}

uint32_t gen_seed_xor()
{
  const uint8_t n = PIN_SAMPLE_SIZE;

  // sample spread of available pins (A0-A7, A9-A15), re-sample A0 so we have 16 reads
  uint16_t pins[n] = {A0,A1,A2,A3,A4,A5,A6,A7,A9,A10,A11,A12,A13,A14,A15,A0};

  // use only low nibble of low byte for larger variability
  // since analogRead() returns a 10 bit wide int, high nibble of high byte always 0000 - discard
  // low nibble of high byte can only be 0000, 0001, 0010, 0011, but realistically is almost always 0001 - discard
  // high nibble of low byte repeats too frequently (4,5,6,7) - discard

  uint16_t res[n];    // 0000 00XX XXXX XXXX    // 10 bit wide int
                      //        XX hnib lnib
  uint8_t lnib[n];    // low nibble of low byte
  uint8_t xnib[n];    // xor'd nibbles
  uint32_t seed=0;

  for (int i=0; i<n; i++)
  {
    res[i] = analogRead(pins[i]);   // analogRead(floating_pin) returns 0 to 1023 (realistically ~ 200 to 500)

    // get low nibbles
    lnib[i] = res[i] & 0x0F;        // 00 0000 1111 (0x0F), mask
  }

  // xor the staggered low nibbles and shift to create a uint32_t seed
  for (int j=0; j<n/2; j++)
  {
    xnib[j] = lnib[j]^lnib[n/2+j];      // nibble xor pairs (0,8), (1,9), ..., (7,15)
  }
  // shift 8 nibbles into a uint32_t seed
  seed = ((uint32_t)xnib[0] << 28) | ((uint32_t)xnib[1] << 24) | ((uint32_t)xnib[2] << 20) | ((uint32_t)xnib[3] << 16) |
                  ((uint32_t)xnib[4] << 12) | ((uint32_t)xnib[5] << 8) | ((uint32_t)xnib[6] << 4) | (uint32_t)xnib[7];
  return seed;
}

void set()
{
  sram = SRAM_CY62256N();
  uint8_t buf[BUF_SIZE] = {'\0'};
  uint32_t seed=0;
  size_t result=0;

  DDRB = 0b11100000; //Pin 13, 12, 11 set to output
  DDRA = 0b11111111; //Pins 22-29 Set as Output (Lower Byte of Address)
  DDRC = 0b01111111; //Pins 37-31 Set as Output (Upper Byte of Address)

  pinMode(PIN_POWER, OUTPUT);

  Serial.begin(115200);         // arduino mega available immediately after init
  delay(1000);
  Serial.setTimeout(10000);     // timeout (ms)

  // TODO: alt method for quality random seed w/o user action - maybe have data on sd card
  serial_print("run the seed_arduino.py script for best quality PRNG seed\r\n");          // prompt user to run script
  serial_print("waiting 10 seconds till fallback to alternate seed generation...\r\n");

  result = Serial.readBytes(buf, BUF_SIZE);     // readBytes() blocks till either buf is filled or timeout
  if ((result == BUF_SIZE) && (buf[0] == (uint8_t)SEED_ARDUINO_CMD))
  {
    // first byte received was SEED command, next four bytes used to compose seed
    seed = ((uint32_t)buf[4] << 24) | ((uint32_t)buf[3] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[1];
    serial_print("arduino seed generated from urandom data\r\n");
  }
  else      // timeout or error
  {
    // fallback if bytes from host are not received
    serial_print("fallback to improved, but still weaker, PRNG seeding method\r\n");
    seed = gen_seed_xor();
    // seed = analogRead(A0);       // very inadequate
  }
  randomSeed(seed);     // begin PRNG stream
}

void decode(uint8_t *key_32) {
  BCH bch;
  Tools tools;
  bch.initialize();

  uint8_t helper_data_padded_new[8 * 37];
  uint8_t xor_reproduction_new[8 * 37];
  uint8_t reconstructed_key_new[37];

  memset(helper_data_padded_new, 0, sizeof(helper_data_padded_new));
  memset(xor_reproduction_new, 0, sizeof(xor_reproduction_new));
  memset(reconstructed_key_new, 0, sizeof(reconstructed_key_new));

  int row = 37;
  int n = bch.get_n();
  int k = bch.get_key_length();

  int k_length_bit = row;//tools.ceil(k*row,8);
  int n_length_bit = row * 8;//tools.ceil(n*row,8);

  long thisItem[7];

  /************************************
   ****** REPRODUCTION PROCEDURE ******
   ************************************/
  /******************** ASSERT HELPER DATA PADDED**********************/
  for (int i = 0; i < row; i++) {
    memcpy(&helper_data_padded_new[i * 8], &helper_data_new[i * 7], 7 * sizeof(uint8_t));
  }

  /******************** ASSERT XOR REPRODUCTION **********************/
  for (int i = 0; i < row; i++) {
    for (int j = 0; j < 8; j++) {
      xor_reproduction_new[i * 8 + j] = puf_binary_new[i * 8 + j] ^ helper_data_padded_new[i * 8 + j];
    }
  }

  /******************** ASSERT RECONSTRUCTED KEY **********************/
  for (int i = 0; i < row; i++) {
    bch.decode_bch(&xor_reproduction_new[i * 8], &reconstructed_key_new[i]);
  }

  tools.convert_key_back(reconstructed_key_new, key_32);
}


void turn_on_sram() {
  analogWrite(PIN_POWER, 255);
}

void turn_off_sram() {
  analogWrite(PIN_POWER, 0);

  for (int i = 22; i <= 29; i++) {
    digitalWrite(i, LOW);
  }
  for (int i = 31; i <= 37; i++) {
    digitalWrite(i, LOW);
  }
  for (int i = 42; i <= 49; i++) {
    digitalWrite(i, LOW);
  }
  for (int i = 11; i <= 13; i++) {
    digitalWrite(i, LOW);
  }
}

void initializeSD() {
  Serial.print("Initializing SD card...");

  if (!SD.begin(7)) {
    Serial.println("initialization failed!");
    delay(2000);
    exit(0);
  }
  Serial.println("initialization done.");
}

boolean isValidNumber(String str) {
  if (str.length() == 0)
    return false;
  for (byte i = 0; i < str.length(); i++)
  {
    if (!isDigit(str.charAt(i)))
      return false;
  }
  return true;
}

String convertString(String a) {
  String s;
  boolean isNum;
  for (byte i = 0; i < a.length(); i++)
  {
    isNum = isDigit(a.charAt(i));
    if (isNum)
      s += a.charAt(i);
  }
  return s;
}

void readSRAMfromMicroSD() {
  memset(puf_binary_new, 0, sizeof(puf_binary_new));
  uint8_t result;

  // read
  int index = 0, i = 0;

  String name = "c.txt";
  String a, b;
  long thisItem;
  File myFile = SD.open(name);
  if (myFile) {
    while (myFile.available()) {
      a = myFile.readStringUntil('\n');
      b = convertString(a);
      if (isValidNumber(b)) {
        thisItem = b.toInt();
        if (readBit(thisItem))
          puf_binary_new[index] = puf_binary_new[index] | 0x1;
        if ((i + 1) % 8 == 0) {
          index++;
        } else {
          puf_binary_new[index] = puf_binary_new[index] << 1;
        }
        i++;
      }
    }
    myFile.close();
  } else {
    Serial.println("error ");
  }
}

void readHelperDatafromMicroSD() {
  memset(helper_data_new, 0, sizeof(helper_data_new));
  uint8_t result;

  // read
  int index = 0, i = 0;

  String name = "h.txt";
  String a, b;
  long thisItem;
  File myFile = SD.open(name);
  if (myFile) {
    while (myFile.available()) {
      a = myFile.readStringUntil('\n');
      b = convertString(a);
      if (isValidNumber(b)) {
        thisItem = b.toInt();
        helper_data_new[i] = thisItem;
        i++;
      }
    }
    myFile.close();
  } else {
    Serial.println("error ");
  }
}

void writeToSD(String file_name, uint8_t* things_to_write, int sz) {
  File myFile = SD.open(file_name,  O_WRITE | O_CREAT | O_TRUNC);
  if (myFile) {
    myFile.write(things_to_write, sz);
    myFile.close();
  } else {
    Serial.println("error writing");
  }
}

void readFromSD(String file_name, uint8_t* result, int sz) {
  File myFile = SD.open(file_name);
  byte a;

  if (myFile) {
    myFile.read(result, sz);
    myFile.close();
  } else {
    Serial.println("error opening");
  }
}

void print_key(uint8_t* key, uint8_t length) {
  for (int i = 0; i < length; i++) {
    Serial.print("0123456789abcdef"[key[i] >> 4]);
    Serial.print("0123456789abcdef"[key[i] & 0xf]);
  }
  Serial.println();
}

void gen_IV(uint8_t * result, uint8_t length)
{
  // moved and modified PRNG seeding code to set() function for proper init

  for (uint8_t i=0;i<length;i++){
    result[i]=random(256);
  }
}

void derive_new_key(String user_password, uint8_t* final_key, uint8_t* key_32) {
  SHA3_256 sha3_256;

  sha3_256.resetHMAC(key_32, 32);
  sha3_256.update(reinterpret_cast<const uint8_t*>(&user_password[0]), user_password.length());
  sha3_256.finalizeHMAC(key_32, 32, final_key, 32);
}

void check_integrity(uint8_t* encrypted, uint8_t* result, uint8_t* key_32) {
  SHA3_256 sha3_256;

  sha3_256.resetHMAC(key_32, 32);
  sha3_256.update(encrypted, sizeof(encrypted));
  sha3_256.finalizeHMAC(key_32, 32, result, 32);
}

void encrypt_test(uint8_t* final_key, uint8_t* iv, uint8_t *plain, uint8_t *result) {
  CTR<AES256> ctraes256;
  ctraes256.clear();
  if (!ctraes256.setKey(final_key, 32)) {
      Serial.print("setKey failed\n");
      return;
  }
  if (!ctraes256.setIV(iv, 16)) {
      Serial.print("setIV failed\n");
      return;
  }

  ctraes256.encrypt(result, plain, 32);
}

void setup(void)
{
  delay(1000);
  String user_password = "";
  uint8_t final_key[32];
  uint8_t key_32[32];
  memset(key_32, 0, sizeof(key_32));
  memset(final_key, 0, sizeof(final_key));

  set();
#ifdef DEBUG
  exit(0);
#endif
  initializeSD();

  turn_on_sram();
  readSRAMfromMicroSD();
  turn_off_sram();
  readHelperDatafromMicroSD();

  /**
     START DECODING - USE THE ERROR CORRECTION - TO CREATE THE PUF KEY
  */
  decode(key_32);
  Serial.println();
  Serial.print("PUF key \t: ");
  print_key(key_32, 32);

  /**
     KEY DERIVATION STAGE
  */
  Serial.println();
  Serial.println("Please enter password to generate the final key");
  while (Serial.available() == 0)
  {
  }
  user_password = Serial.readString();
  Serial.print("User password \t: ");
  Serial.println(user_password);

  derive_new_key(user_password, final_key, key_32);

  Serial.print("Final key \t: ");
  print_key(final_key, 32);

  /**
     ENCRYPT USING THE FINAL KEY
  */
  Serial.println();
  Serial.println("Please enter user's key (256-bit) to be encrypted");
  while (Serial.available() == 0)
  {
  }
  String plain_text = Serial.readString();
  Serial.print("plaintext \t: ");
  Serial.println(plain_text);
  Serial.begin(115200);

  Serial.println();

  uint8_t result[32];
  memset(result, 0, sizeof(result));

  uint8_t iv[16];
  memset(iv, 0, sizeof(iv));
  gen_IV(iv, 16);
  // Serial.print("IV \t\t: ");
  // print_key(iv, 16);

  encrypt_test(final_key, iv, reinterpret_cast<const uint8_t*>(&plain_text[0]), result);
  Serial.print("encrypted \t: ");
  print_key(result, 32);

  uint8_t h_result[32];
  memset(h_result, 0, sizeof(h_result));
  check_integrity(result, h_result, key_32);

  writeToSD("e.txt", result, 32);
  writeToSD("i.txt", iv, 16);
  writeToSD("h.txt", h_result, 32);
}

void loop() {
  delay(1000);
  exit(0);
}
