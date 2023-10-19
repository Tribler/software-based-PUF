# This file is part of the software-based-PUF,
# https://github.com/Tribler/software-based-PUF
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
from PUF import SerialPUF, Tools

# Temporarily append puf_xtra package path.  New modules of reused code are organized in the puf_xtra package
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'puf_xtra'))

from conf import conf, const        # double import
import portutils
import bindutils

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
    :param serialPUF: an instance of the SerialPUF class
    :param delay: delay value
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
    def __init__(self, thread_name, is_sram_23lc1024, serialconnection, bitrate, index='', goal=2331,
                 initial_delay=0.33, step_delay=0.01):
        threading.Thread.__init__(self, name=thread_name)
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


# operation mode is set in configuration file, conf.py
if conf.OPERATION_MODE == 'MONO':
    # **************** generate bits, single serial device with auto detection ****************
    # detect() will automatically set 'serialconnection' param when possible
    # user may set a device manually via SERIAL_DEVICE in conf.py but should usually not be necessary
    # detection progression:  configuration file --> previous selected device --> MEGA2560_R3 (arduino) -->
    # MEGA2560_R3 (clone) --> display serial devices (user select and save)

    s = const.RUN
    while s == const.RUN:       # runs again if detect() returns RERUN          (RERUN == RUN)
        s = portutils.detect()
    if s:       # s not empty
        mono_idx = ''
        mono_unique_id = bindutils.get_uid(s)
        if mono_unique_id:
            mono_idx = bindutils.get_index(conf.BIND_FILE, unique_id=mono_unique_id)
        else:
            print('get_uid error\n')
            sys.exit(2)
        if mono_idx:
            thread = StableBitsGenerator(thread_name='thread0', is_sram_23lc1024=False, serialconnection=s,
                                         bitrate=conf.BITRATE, index=mono_idx)
            thread.start()
        else:
            print('get_index error... exit\n')
            sys.exit(3)
    else:
        print('device not found... exit')
        sys.exit(1)

# operation mode is set in conf.py
elif conf.OPERATION_MODE == 'PARALLEL' and conf.USE_BINDING:
    # **************** parallelly generate stable bits ****************
    # must set devices in the configuration file via SERIAL_DEVICE param (use device paths)
    # creates a thread for each device path given in configuration list
    # each device unique id is bound to an index value and the index value is set accordingly
    # for each thread(device) so the user no longer needs to set manually

    # check to make sure we can get all unique_id and index values without error
    n = len(conf.SERIAL_DEVICE)             # number of serial devices
    unique_id = ['' for _ in range(n)]      # empty n item list
    idx = ['' for _ in range(n)]            # empty n item list
    err=0

    # added GET_UID case to Arduino code for cy62256nll and 23lc1024
    for i in range(n):
        unique_id[i] = bindutils.get_uid(conf.SERIAL_DEVICE[i])     # gets unique_id
        if unique_id[i] == '':      # break on error
            err += 1
            break
        idx[i] = bindutils.get_index(conf.BIND_FILE, unique_id=unique_id[i])    # now get the index (created if absent)
        if idx[i] == '':        # error, increment error counter and break
            err += 1
            break
    if err > 0:
        print('error getting device unique id and/or index binding\n'
              'unable to proceed, exit')
        sys.exit(1)

    # all serial devices are properly bound, all should succeed generating stable bits
    threads=[]
    for i in range(len(conf.SERIAL_DEVICE)):
        tname = 'thread' + str(i)
        thread = StableBitsGenerator(thread_name=tname, is_sram_23lc1024=False, serialconnection=conf.SERIAL_DEVICE[i],
                                     bitrate=conf.BITRATE, index=idx[i])
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()       # wait until all threads finish

    # threads will execute concurrently but all code added hereafter is blocked until threads finish
    print('all threads finished')
    sys.exit(0)

elif conf.OPERATION_MODE == 'PARALLEL' and conf.USE_LEGACY:
    # ************************* legacy parallel *************************
    # set USE_BINDING to False in conf.py if you want to use legacy code
    # user must set all parameters manually
    thread1 = StableBitsGenerator(thread_name='thread1', is_sram_23lc1024=False,
                                  serialconnection='/dev/cu.usbmodem14111', bitrate=115200, index='A')
    thread1.start()
    thread2 = StableBitsGenerator(thread_name='thread2', is_sram_23lc1024=False,
                                  serialconnection='/dev/cu.usbmodem14121', bitrate=115200, index='C')
    thread2.start()

# **************** solo generating stable bits ****************
# thread1 = stableBitsGenerator(is_sram_23lc1024=False, serialconnection='/dev/cu.usbmodem14111', bitrate=115200,
#                               index='A')
# thread1.start()
