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
from PUF import SerialPUF       # Tools

# Temporarily append puf_xtra package path.  New modules and config files are organized in the puf_xtra package.
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'puf_xtra'))   # add puf_xtra path

import bindutils
import portutils
from conf import conf, const

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
    def __init__(self, thread_name, serialconnection, bitrate, index, goal=2331, initial_delay=0.33, step_delay=0.01):
        threading.Thread.__init__(self, name=thread_name)
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


# detect() will automatically set 'serialconnection' param when possible
# user may set a device manually via SERIAL_DEVICE in conf.py but should usually not be necessary
# detection progression:  configuration file --> previous selected device --> MEGA2560_R3 (arduino) -->
# MEGA2560_R3 (clone) --> display serial devices (user select and save)

s = const.RUN
while s == const.RUN:       # runs again if detect() returns RERUN          (RERUN == RUN)
    s = portutils.detect()
if s:   # s not empty
    idx = ''
    unique_id = bindutils.get_uid(s)
    if unique_id:
        idx = bindutils.get_index(conf.BIND_FILE, unique_id=unique_id)
    else:
        print('get_uid error\n')
        sys.exit(2)
    if idx:
        thread = EnrollmentTools(thread_name='thread0', serialconnection=s, bitrate=conf.BITRATE, index=idx,
                                 initial_delay=0.31)
        thread.start()
    else:
        print('get_index error... exit\n')
        sys.exit(3)
else:
    print('device not found... exit')
    sys.exit(1)

#thread1 = threading.Thread(target=EnrollmentTools, kwargs={'serialconnection':s, 'bitrate':conf.BITRATE,
#                                                           'index':"C", 'initial_delay':0.31})
#thread1.start()
