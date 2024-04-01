#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include <stdio.h>
#include <SRAM.h>

#define TURN_OFF_CMD 40
#define TURN_ON_CMD 41
#define READ_PAGE_CMD 42
#define READ_SINGLE_CMD 43
#define WRITE_ALL_ONE_CMD 44
#define WRITE_ALL_ZERO_CMD 45
#define WRITE_ANALOG 46
#define WRITE_PAGE 47
#define WRITE_SINGLE 48
#define READ_BIT_CMD 49
#define GET_UID 55
#define GET_URANDOM_SEED 57
#define SEED_ARDUINO_CMD 0x39           // flag preceding incoming seed data
#define UID_BUF_SIZE 36
#define BUF_SIZE 5
#define PIN_SAMPLE_SIZE 16

#define PIN_POWER_ANALOG A8
#define PIN_POWER 9

SRAM sram;

int count = 0;
char command[6];
uint32_t g_seed;

void writeVoltage(uint8_t pin, float value){
  analogWrite(pin, value * 51);
}

float readVoltage(uint8_t pin){
  uint8_t r = analogRead(pin);
  return r / 51;
}

void writeAnalog(uint8_t pin, uint8_t value){
  analogWrite(pin, value);
}

int readAnalog(uint8_t pin){
  return analogRead(pin);
}

void randomVoltage(uint8_t pin){
  analogWrite(pin, random(130, 255));
}

void offVoltage(uint8_t pin){
  analogWrite(pin, 0);
}

void onVoltage(uint8_t pin){
  analogWrite(pin, 255);
}

void print_binary(int v, int num_places)
{
    int mask=0, n;
    String a = "";

    for (n=1; n<=num_places; n++)
    {
        mask = (mask << 1) | 0x0001;
    }
    v = v & mask;  // truncate v to specified number of places

    while(num_places)
    {
        a += (v & (0x0001 << (num_places-1))) ? "1" : "0";
        --num_places;
    }
    Serial.println(a);
    Serial.flush();
}

boolean readBit(long location){
  uint8_t result = sram.read_single(floor(location / 8));
  return result >> (7 - (location % 8)) & 0x1 == 1;
}

uint32_t gen_seed_xor()
{
  const uint8_t n = PIN_SAMPLE_SIZE;

  // sample spread of available pins (A0-A7, A9-A15), re-sample A0 so we have 16 reads
  uint16_t pins[n] = {A0,A1,A2,A3,A4,A5,A6,A7,A9,A10,A11,A12,A13,A14,A15,A0};

  // pin A8 is wired (not floating) - don't use
  // use only low nibble of low byte for larger variability
  // since analogRead() returns a 10 bit wide int, high nibble of high byte always 0000 - discard
  // low nibble of high byte can only be 0000, 0001, 0010, 0011, but realistically is almost always 0001 - discard
  // high nibble of low byte repeats too frequently (4,5,6,7) - discard

  uint16_t res[n];    // 0000 00XX XXXX XXXX    // 10 bit wide int
                      //        XX hnib lnib
  uint8_t lnib[n];    // low nibble of low byte
  uint8_t xnib[n];    // xor'd nibbles
  uint32_t seed;

  for (int i=0; i<n; i++)
  {
    res[i] = analogRead(pins[i]);   // analogRead(floating_pin) returns 0 to 1023 (realistically ~ 200 to 500)
    lnib[i] = res[i] & 0x0F;        // get low nibbles, 00 0000 1111 (0x0F), mask
  }

  // xor the staggered low nibbles and shift to create a uint32_t seed
  for (int j=0; j<n/2; j++)
  {
    xnib[j] = lnib[j]^lnib[n/2+j];      // xor nibble pairs (0,8), (1,9), ..., (7,15)
  }
  // shift 8 nibbles into a uint32_t seed
  seed = ((uint32_t)xnib[0]<<28) | ((uint32_t)xnib[1]<<24) | ((uint32_t)xnib[2]<<20) | ((uint32_t)xnib[3]<<16) |
                  ((uint32_t)xnib[4]<<12) | ((uint32_t)xnib[5]<<8) | ((uint32_t)xnib[6]<<4) | (uint32_t)xnib[7];
  return seed;
}

void setup(void)
{
  uint8_t buf[BUF_SIZE] = {'\0'};
  uint32_t seed=0;
  size_t result=0;

  Serial.begin(115200);     // mega board serial is available immediately after init
  delay(1000);
  Serial.setTimeout(5000);  // timeout (ms)

  // TODO: fix external seeding
  // read CMD + urandom data sent from host (initiated in python script)

  /*result = Serial.readBytes(buf, BUF_SIZE);     // readBytes() blocks till either buf is filled or timeout
  if ((result == BUF_SIZE) && (buf[0] == (uint8_t)SEED_ARDUINO_CMD))
  {
    // first byte received was seeding command, next four bytes used to compose seed
    seed = ((uint32_t)buf[4] << 24) | ((uint32_t)buf[3] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[1];
    Serial.println("arduino seed generated from urandom data");
  }
  else      // timeout or error
  {
    // fallback if bytes from host are not received
    Serial.println("fallback to improved, but still weaker, PRNG seeding")
    seed = gen_seed_xor();
  }*/

  // MUST use on device seeding for now
  seed = gen_seed_xor();
  randomSeed(seed);     // begin PRNG stream

  // analogRead(floating_pin) isn't remotely close to random, we need another approach
  // w/o dedicated onboard HWRNG, high entropy on embedded is nearly impossible
  // partial fix:  combined multi pin reads into uint32_t seed using gen_seed_xor()

  sram = SRAM();
  sram.set_pin_cs(10);
  sram.set_pin_hold(11);
  sram.set_pin_mosi(51);
  sram.set_pin_miso(50);
  sram.set_pin_power(7);
  sram.set_pin_sck(52);

  pinMode(PIN_POWER, OUTPUT);
}

void send_command(char c){
  Serial.write(c);
  Serial.write('\r');
  Serial.write('\n');
}

uint8_t result;
unsigned long address;      // uint32_t
uint8_t result_page[32];

void check_command()
{
  uint16_t q;
  if (command[0] != 99)
    return;
  switch (command[1])
  {
    case TURN_OFF_CMD:  // turn_off
      sram.turn_off();
      writeAnalog(PIN_POWER, 0);
      if (command[2] > 0)
      {
        delay(1000);
      }
      q = readAnalog(PIN_POWER_ANALOG);

      Serial.write(99);
      Serial.write(TURN_OFF_CMD);
      Serial.write((q >> 8) & 0xFF);
      Serial.write(q & 0xFF);
      for (int i=0; i < 36-4; i++)
        Serial.write(0);
      break;

    case TURN_ON_CMD:  // turn_on
      sram.turn_on();
      writeAnalog(PIN_POWER, 255);
      if (command[2] > 0)
      {
        delay(1000);
      }
      q = readAnalog(PIN_POWER_ANALOG);

      Serial.write(99);
      Serial.write(TURN_ON_CMD);
      Serial.write((q >> 8) & 0xFF);
      Serial.write(q & 0xFF);
      for (int i=0; i < 36-4; i++)
        Serial.write(0);
      break;

    case READ_PAGE_CMD:  //
      memset(result_page, 0 , sizeof(result_page));
      address = 0;
      memcpy(&address, &command[2], 2);
      sram.read_page(address, result_page);

      Serial.write(99);
      Serial.write(READ_PAGE_CMD);
      Serial.write((address >> 8) & 0xFF);
      Serial.write(address & 0xFF);

      for (int i=0;i<32;i++)
      {
        Serial.write(result_page[i]);
      }
      break;

    case READ_SINGLE_CMD:       // read single byte at address
      address = 0;
      memcpy(&address, &command[2], 4);
      result = sram.read_single(address);

      Serial.write(99);
      Serial.write(READ_SINGLE_CMD);
      Serial.write((address >> 24) & 0xFF);
      Serial.write((address >> 16) & 0xFF);
      Serial.write((address >> 8) & 0xFF);
      Serial.write(address & 0xFF);
      Serial.write(result);

      for (int i=0; i < 36-7; i++)
        Serial.write(0);
      break;

    case READ_BIT_CMD:      // read single bit from byte at address
      boolean c;
      memcpy(&address, &command[2], 4);
      c = readBit(address);

      Serial.write(99);
      Serial.write(READ_BIT_CMD);
      Serial.write((uint8_t) (address >> 24) & 0xFF);
      Serial.write((uint8_t) (address >> 16) & 0xFF);
      Serial.write((uint8_t) (address >> 8) & 0xFF);
      Serial.write(address & 0xFF);
      Serial.write(c ? 1 : 0);
      for (int i=0; i < 36-7; i++)
        Serial.write(0);
      break;

    case WRITE_ALL_ONE_CMD:
      sram.write_all_one();
      delay(500);
      Serial.write(99);
      Serial.write(WRITE_ALL_ONE_CMD);
      Serial.write(1);
      for (int i=0; i < 36-3; i++)
        Serial.write(0);
      break;

    case WRITE_ALL_ZERO_CMD:
      sram.write_all_zero();
      delay(500);
      Serial.write(99);
      Serial.write(WRITE_ALL_ZERO_CMD);
      for (int i=0; i < 36-2; i++)
        Serial.write(0);
      break;

    case WRITE_ANALOG:
      q = (uint16_t) command[2];
      Serial.write(99);
      Serial.write(WRITE_ANALOG);
      Serial.write(q & 0xFF);
      analogWrite(PIN_POWER, q & 0xFF);
      q = analogRead(PIN_POWER_ANALOG);
      sram.turn_on();
      Serial.write((q >> 8) & 0xFF);
      Serial.write(q & 0xFF);
      for (int i=0; i < 36-5; i++)
        Serial.write(0);
      break;

    case WRITE_PAGE:
      address = 0;
      memcpy(&address, &command[2], 2);
      sram.write_page(address, command[4] == 1);
      Serial.write(99);
      Serial.write(WRITE_PAGE);
      Serial.write((address >> 8) & 0xFF);
      Serial.write(address & 0xFF);
      for (int i=0; i < 36-4; i++)
        Serial.write(0);
      break;

    case WRITE_SINGLE:
      address = 0;
      memcpy(&address, &command[2], 4);
      // sram.write_single(address, );
      Serial.write(99);
      Serial.write(WRITE_SINGLE);
      Serial.write((address >> 24) & 0xFF);
      Serial.write((address >> 16) & 0xFF);
      Serial.write((address >> 8) & 0xFF);
      Serial.write(address & 0xFF);
      for (int i=0; i < 36-6; i++)
        Serial.write(0);
      break;

    case GET_URANDOM_SEED:      // get a crypto quality uint32_t seed value from host
      g_seed = (command[5] | (command[4] << 8) | (command[3] << 16) | (command[2] << 24));
      break;

    case GET_UID:       // uses ArduinoUniqueID library to retrieve uid
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
