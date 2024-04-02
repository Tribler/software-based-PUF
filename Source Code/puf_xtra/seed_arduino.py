# This file is an optional arduino seeding script and is part of
# software-based-PUF, https://github.com/Tribler/software-based-PUF
# Copyright (C) 2024 myndcryme
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import serial
import time
import struct
import portutils
from conf import conf, const

def seeder():
    """
    Function is guarded since it is intended to be run directly.
    This function generates a crypto quality seed value (4 bytes) from the host's
    urandom pool and transmits the data via serial to the connected Arduino.
    Running this script is an optional but highly recommended to remedy inadequate
    PRNG seeding.
    :return: void
    """
    def get_urandom_seed(source: str='OS_URANDOM'):
        """
        Nested function reads four bytes of urandom data from source and returns
        the composed seed value.
        :param source: source for urandom data
        :return: result (for use seeding randomSeed() on arduino), None
        """

        result=None
        if source == 'OS_URANDOM':      # use python built-in (cross platform)
            data = os.urandom(4)        # 4 bytes to make a uint32_t
            result = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3]
        elif source == 'DEV_URANDOM':     # linux /dev/urandom pool read
            with open('/dev/urandom','rb') as f:
                data = f.read(4)
                result = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3]
        return result

    s = const.RUN
    while s == const.RUN:
        s = portutils.detect()  # get device path so we can send the seed value to the arduino
    if s:                       # device path acquired
        ser=None
        t=time.time()
        print("waiting for serial connection", end="")
        while time.time() - t < 30:     # wait for arduino with timeout (30 seconds)
            try:
                ser = serial.Serial(s, conf.BITRATE, timeout=1)
                break       # connected
            except serial.SerialException:      # no connection (yet)
                print(".", end="")
                time.sleep(0.5)

        if ser is not None and ser.isOpen():
            seed = get_urandom_seed(conf.URANDOM_SOURCE)
            if seed is not None:
                ser.write(struct.pack('<BI', conf.SEED_ARDUINO_CMD, seed))  # pack binary (L-endian, uint8_t, uint32_t)
                print("\nseed generated and written to arduino port")
            ser.close()
        else:
            print("\ncould not establish connection, exiting...")
            exit(1)
    else:
        print("\ndevice not found, exit")
        exit(2)
    exit(0)

if __name__ == '__main__':        # main guard, only allow direct script execution
    seeder()