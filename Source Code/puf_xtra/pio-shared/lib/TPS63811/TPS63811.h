// This file is part of the software-based-PUF,
// https://github.com/Tribler/software-based-PUF
// Copyright (C) 2024 myndcryme
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

#ifndef TPS63811_H
#define TPS63811_H

/* TPS63811 constants */
#define TPS63811_ADDR 0x75      // I2C address
#define CONTROL 0x01            // control register address
#define STATUS 0x02             // status register addr
#define VOUT1 0x04              // vout1 register addr
#define VOUT2 0x05              // vout2 register addr
#define VOUT1_RESET 0x3C        // vout1 reg default (adds 1.5v to vout1 min)
#define VOUT2_RESET 0x42        // vout2 reg default (adds 1.65v to vout2 min)
#define VOUT_INCREMENT 0.025    // tps63811 output voltage increment
#define CONTROL_INIT 0x4A       // control register bits for init
#define RH_MIN 2.025            // range bit HIGH, min voltage output
#define RH_MAX 5.200            // range bit HIGH, max voltage output
#define RL_MIN 1.800            // range bit LOW, min voltage output
#define RL_MAX 4.975            // range bit LOW, max voltage output

/* dynamic voltage scaling slew rate constants */
/*             0b000000xx <- xx are control register slew bits */
#define DVS_1  0b00000000  1.0 V/ms
#define DVS_2  0b00000001  2.5 V/ms
#define DVS_5  0b00000010  5.0 V/ms
#define DVS_10 0b00000011  10.0 V/ms

#define PIN_TPS_PWR 7

/* BitIndex struct holds the bit position(s) each field occupies within a register and
 * is used by register clr/set functions to create masks for bit manipulation.  Declaration here,
 * assignment in definition.
 */
struct BitIndex {
  /* control register fields */
	const uint8_t range[1];
	const uint8_t enable[1];
	const uint8_t fpwm[1];
	const uint8_t rpwm[1];
	const uint8_t slew[2];

	/* vout register fields */
	const uint8_t vout1[7];
	const uint8_t vout2[7];
};

extern const struct BitIndex bx;

/* function declarations */
uint8_t wire_read(uint8_t addr, uint8_t *val);
uint8_t wire_write(uint8_t addr, uint8_t val);
uint8_t register_clr_bit(uint8_t addr, const uint8_t field[], size_t n);
uint8_t register_set_bit(uint8_t addr, const uint8_t field[], size_t n, const uint8_t *val=nullptr);
uint8_t calc_vout_regval(float vout);

uint8_t disable_output_converter();
uint8_t enable_output_converter();

void tps63811_on(int pin=PIN_TPS_PWR);
void tps63811_off(int pin=PIN_TPS_PWR);
uint8_t tps63811_init(uint8_t val=CONTROL_INIT);

#endif