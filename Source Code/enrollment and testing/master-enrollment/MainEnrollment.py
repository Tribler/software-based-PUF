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

import sys
import os
import time
import threading
from random import shuffle
from PUF import SerialPUF, Tools

# Append puf_xtra package path.  New modules of reused code are organized in the puf_xtra package.
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'puf_xtra'))   # add puf_xtra path
import port_detect as pd


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
    """
    Locate stable bits using data remanence approach as the bit selection algorithm
    :param serialPUF: serial connection
    :param goal: number of bits need to be generated
    :param initial_delay: 
    :param step_delay:
    :return:
    """
    a = get_strong_bits_by_goal(serialPUF, goal, initial_delay, step_delay, write_ones=False)
    b = get_strong_bits_by_goal(serialPUF, goal, a[1] - step_delay, step_delay, write_ones=True)
    strong_ones = a[0]
    strong_zeros = b[0]
    return [strong_ones, strong_zeros]


class EnrollmentTools(threading.Thread):
    def __init__(self, serialconnection, bitrate, index, goal=2331, initial_delay=0.33, step_delay=0.01):
        threading.Thread.__init__(self)
        self.serialconnection = serialconnection
        self.bitrate = bitrate
        self.index = index
        self.goal = goal
        self.initial_delay = initial_delay
        self.step_delay = step_delay

    def run(self):
        # Initialize serial connection to connect to Arduino
        serialPUF = SerialPUF.SerialPUF()
        if not serialPUF.connect(self.serialconnection, self.bitrate):
            print("Error connecting to Arduino")
            exit(1)
        time.sleep(2)

        serialPUF.check_if_it_is_working()

        # -------------------- DATA REMANENCE TRIAL --------------------------------------
        a = get_strong_bits(serialPUF, self.goal, self.initial_delay, self.step_delay)
        strong_ones = a[0]
        strong_zeros = a[1]

        # -------------------- GENERATE CHALLENGES ---------------------------------------
        x = []
        x.extend(strong_zeros[:2331])
        x.extend(strong_ones[:2331])
        x = list(map(int, x))

        shuffle(x)
        serialPUF.write_challenges_to_sd(x[:37 * 63])
        # Tools.save_to_file(x[:37 * 63], "challenge-" + self.index + ".txt", with_comma=True)

        serialPUF.generate_helper_data_on_sd()


# Device auto detected or may be selected by user
# serialconnection param may be set manually w/ device path or VID:PID (but not required)
# examples :     s = pd.detect()     s = pd.detect('/dev/ttyACM0')     s = pd.detect('2341:0042')
#
s = pd.detect()  # user may change this but isn't necessary if using genuine ArduinoMEGA2560_R3 (examples above)
if s != '':
    thread1 = EnrollmentTools(serialconnection=s,bitrate=115200,index="C",initial_delay=0.31)
    thread1.start()
else:
    print('device not found... exit')
    sys.exit()

# ***** open an issue and provide your working Arduino clone VID:PID to add for auto detection *****
