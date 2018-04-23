#include <Arduino.h>
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
#define PIN_POWER_ANALOG A8
#define PIN_POWER 9

SRAM sram;

int count = 0;
char command[6];

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

void setup(void)
{
  Serial.begin(115200);

  randomSeed(analogRead(A0));

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
unsigned long address;
uint8_t result_page[32];

void check_command(){
  uint16_t q;
  if (command[0] != 99)
    return;
  switch (command[1]) {
    case TURN_OFF_CMD:  // turn_off
      sram.turn_off();
      writeAnalog(PIN_POWER, 0);
      if (command[2] > 0){
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
      if (command[2] > 0){
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

      for (int i=0;i<32;i++){
        Serial.write(result_page[i]);
      }
      break;
    case READ_SINGLE_CMD:  // turn_on
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
    case READ_BIT_CMD:
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
  if (Serial.available() > 0) {
    command[count] = Serial.read();
    count++;
    if (count == 6){
      check_command();
      count = 0;
    }
  }
}
