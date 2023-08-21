# This file is part of the software-based-PUF,
# https://github.com/Tribler/software-based-PUF
#
# Modifications and additions Copyright (C) 2023 myndcryme
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

# from __future__ import print_function     # for python2
import sys
import os
import time
import threading
# import _thread
from PUF import SerialPUF, Tools

# Append puf_xtra package path.  New modules of reused code are organized in the puf_xtra package.
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'puf_xtra'))   # add puf_xtra path
import port_detect as pd
import puf_consts as pc


def get_strong_bits_by_goal(serialPUF, goal, initial_delay=0.3, step_delay=0.005, write_ones=True):
    """
    Bit selection algorithm using data remanence approach
    :param goal: number of strong bits desired
    :param initial_delay: initial delay
    :param step_delay: initial step delay
    :param write_ones: write one to all location. Set to true if strong zeroes are the goal.
            Otherwise set to false
    :return: location of strong bits
    """
    delay = initial_delay
    strong_bits = []

    current_goal = 0
    delays = []

    while current_goal < goal:
        print("DATA REMANENCE - delay: " + str(delay), end=' : ')
        delays.append(delay)

        result = serialPUF.try_data_remanence(delay, write_ones)
        strong_bits = result[3]
        if not write_ones:
            current_goal = result[2]
        else:
            current_goal = result[1]
        print(current_goal)

        if current_goal < goal:
            delay = delay + step_delay

    return [strong_bits, delay]


def get_strong_bits_by_time(serialPUF, delay, write_ones=True):
    """
    Bit selection algorithm using data remanence approach
    :param goal: number of strong bits desired
    :param initial_delay: initial delay
    :param step_delay: initial step delay
    :param write_ones: write one to all location. Set to true if strong zeroes are the goal.
            Otherwise set to false
    :return: location of strong bits
    """
    delays = []

    print("DATA REMANENCE - delay: " + str(delay), end=' : ')
    delays.append(delay)

    result = serialPUF.try_data_remanence(delay, write_ones)
    strong_bits = result[3]
    if not write_ones:
        current_goal = result[2]
    else:
        current_goal = result[1]
    print(current_goal)

    return strong_bits


def get_strong_bits(serialPUF, goal, initial_delay=0.3, step_delay=0.005):
    a = get_strong_bits_by_goal(serialPUF, goal, initial_delay, step_delay, write_ones=False)
    b = get_strong_bits_by_goal(serialPUF, goal, a[1], step_delay, write_ones=True)
    strong_ones = a[0]
    strong_zeros = b[0]
    return [strong_ones, strong_zeros]


class StableBitsGenerator(threading.Thread):
    def __init__(self, is_sram_23lc1024, serialconnection, bitrate, index='', goal=2331, initial_delay=0.33,
                 step_delay=0.01):
        threading.Thread.__init__(self)
        self.is_sram_23lc1024 = is_sram_23lc1024
        self.serialconnection = serialconnection
        self.bitrate = bitrate
        self.index = index
        self.goal = goal
        self.initial_delay = initial_delay
        self.step_delay = step_delay

    def run(self):
        # **************** Initialize serial connection to connect to Arduino ****************
        serialPUF = SerialPUF.SerialPUF()
        if not serialPUF.connect(self.serialconnection, self.bitrate, is_sram_23lc1024=self.is_sram_23lc1024):
            print("Error connecting to Arduino")
            exit(1)
        time.sleep(2)

        # serialPUF.check_if_it_is_working()

        # strong_ones = get_strong_bits_by_goal(serialPUF, goal=self.goal, initial_delay=self.initial_delay,
        #                                       step_delay=self.step_delay,
        #                                       write_ones=False)
        # # strong_ones = get_strong_bits_by_time(delay=0.31, write_ones=False)
        # strong_zeros = get_strong_bits_by_goal(serialPUF, goal=self.goal, initial_delay=self.initial_delay,
        #                                        step_delay=self.step_delay,
        #                                        write_ones=True)
        # strong_zeros = get_strong_bits_by_time(delay=0.31, write_ones=True)

        a = get_strong_bits(serialPUF, self.goal, self.initial_delay, self.step_delay)
        strong_ones = a[0]
        strong_zeros = a[1]

        x = []
        x.extend(strong_zeros[:2331])
        x.extend(strong_ones[:2331])
        x = list(map(int, x))

        filename = "strongbits-" + self.index + ".txt" if self.index else "strongbits.txt"
        Tools.save_to_file(x, filename, with_comma=True)


# ***** set bit generation mode *****
mode = pc.mode.DEFAULT          # default mode is a single Arduino profiling
# mode = pc.mode.PARALLEL       # set this mode for parallel profiling

if mode == pc.mode.DEFAULT:
    # **************** generate bits, single serial device with auto detection ****************
    # Device auto detected or may be selected by user
    # serialconnection param may be set manually w/ device path or VID:PID (but not required)
    #
    # examples :  s = pd.detect()     s = pd.detect('/dev/ttyACM0')     s = pd.detect('2341:0042')

    s = pd.detect()  # may be set manually by the user but is not necessary (see examples above)
    if s != '':
        print('using device path ' + s)
        thread2 = StableBitsGenerator(is_sram_23lc1024=False, serialconnection=s, bitrate=115200, index='C')
        thread2.start()
    else:
        print('device not found... exit')
        sys.exit()

# ***** open an issue and provide your working Arduino clone VID:PID to add for auto detection *****

elif mode == pc.mode.PARALLEL:
    # TODO: add auto detection for parallel bit generation
    # **************** parallelly generate stable bits ****************
    thread1 = StableBitsGenerator(is_sram_23lc1024=False, serialconnection='/dev/cu.usbmodem14111', bitrate=115200,
                                  index='A')
    thread1.start()
    thread2 = StableBitsGenerator(is_sram_23lc1024=False, serialconnection='/dev/cu.usbmodem14121', bitrate=115200,
                                  index='C')
    thread2.start()

# **************** solo generating stable bits ****************
# thread1 = stableBitsGenerator(is_sram_23lc1024=False, serialconnection='/dev/cu.usbmodem14111', bitrate=115200,
#                               index='A')
# thread1.start()
