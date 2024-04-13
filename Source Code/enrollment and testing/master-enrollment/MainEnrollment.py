# This file is part of the software-based-PUF,
# https://github.com/Tribler/software-based-PUF#
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

# temporarily append puf_xtra package if not already in sys.path
pkg = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'puf_xtra'))
if Tools.sys_path_append(pkg) != 0:
    print("error appending path")
    sys.exit(1)
import bindutils
import portutils
from conf import conf, const

#TODO: add serial polling monitor and send urandom data (if using 23LC1024) when connects

def get_strong_bits_by_goal(serialPUF, goal, initial_delay=0.3, step_delay=0.005, write_ones=True):
    """
    Bit selection algorithm using data remanence approach
    :param serialPUF: an instance of the SerialPUF class
    :param goal: number of strong bits desired
    :param initial_delay: initial delay
    :param step_delay: initial step delay
    :param write_ones: write one to all location. Set to true if strong zeroes are the goal.
            Otherwise set to false
    :return: location of strong bits
    """
    def scale():
        """
        Gets the number of decimal places (scale) in 'step_delay' param.  This is used to round
        'delay' value, avoiding floating point addition errors.
        :return: number of decimal places
        """
        return len(str(step_delay).split('.')[1]) if '.' in str(step_delay) else 0

    delay = initial_delay
    strong_bits = []

    current_goal = 0
    delays = []
    d = scale()
    fmt_spec = '.' + str(d) + 'f'

    while current_goal < goal:
        delay = round(delay, d)
        print("DATA REMANENCE - delay:", format(delay, fmt_spec), end=' : ')
        delays.append(delay)

        result = serialPUF.try_data_remanence(delay, write_ones)
        strong_bits = result[3]
        if not write_ones:
            current_goal = result[2]
        else:
            current_goal = result[1]
        print(current_goal)

        if current_goal < goal:
            delay += step_delay

    return [strong_bits, delay]


def get_strong_bits_by_time(serialPUF, delay, write_ones=True):
    """
    Bit selection algorithm using data remanence approach
    :param serialPUF: an instance of the SerialPUF class
    :param delay: initial delay
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

    # TODO: check if (a[1] - step_delay) is correct since get_strong_bits elsewhere uses just a[1].
    # TODO: not sure why we would want to record strong_ones beginning with:  delay == initial_delay
    # TODO: but record strong_zeros beginning with:  delay == (initial_delay - step_delay)
    # TODO: likely an error

    # b = get_strong_bits_by_goal(serialPUF, goal, a[1] - step_delay, step_delay, write_ones=True)
    b = get_strong_bits_by_goal(serialPUF, goal, a[1], step_delay, write_ones=True)

    strong_ones = a[0]      # a[0] is the strong_bits list, a[1] is the delay
    strong_zeros = b[0]     # ditto
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
        serialPUF.write_challenges_to_sd(x[:37 * 63])       # 2331
        # Tools.save_to_file(x[:37 * 63], "challenge-" + self.idx + ".txt", with_comma=True)

        serialPUF.generate_helper_data_on_sd()


# detect() will automatically set 'serialconnection' param when possible
# user may set a device manually via SERIAL_DEVICE in conf.py but should usually not be necessary
# detection progression:  configuration file --> previous selected device --> MEGA2560_R3 (arduino) -->
# MEGA2560_R3 (clone) --> display serial devices (user select and save)

s = const.RUN
while s == const.RUN:       # runs again if detect() returns RERUN          (RERUN == RUN)
    s = portutils.detect()
if s:   # device found
    idx = bindutils.get_sram_index(s)
    if not idx.startswith("ERR"):
        thread = EnrollmentTools(thread_name='thread0', serialconnection=s, bitrate=conf.BITRATE, index=idx,
                                 initial_delay=0.31)
        thread.start()
        thread.join()       # block caller till thread finishes
    else:
        # TODO: add check / sram profiler
        print("sram did not match any existing profiles - try running GetStableBits.py to generate an sram "
              "profile with index binding before running this script again... exiting")
        sys.exit(2)
else:
    print('device not found... exit')
    sys.exit(1)
print("Enrollment Tools finished")
sys.exit(0)

# TODO: add PARALLEL case

#thread1 = threading.Thread(target=EnrollmentTools, kwargs={'serialconnection':s, 'bitrate':conf.BITRATE,
#                                                           'idx':"C", 'initial_delay':0.31})
#thread1.start()

#begin ARCHIVE_B1
    # idx = ''
    # unique_id = bindutils.get_uid(s)
    # if unique_id:
    #     idx = bindutils.get_index(conf.BIND_FILE, unique_id=unique_id)
    # else:
    #     print('get_uid error\n')
    #     sys.exit(2)
    # if idx:
    #     thread = EnrollmentTools(thread_name='thread0', serialconnection=s, bitrate=conf.BITRATE, index=idx,
    #                              initial_delay=0.31)
    #     thread.start()
    # else:
    #     print('get_index error... exit\n')
    #     sys.exit(3)
#end