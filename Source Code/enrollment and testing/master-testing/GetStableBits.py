# This file is part of the software-based-PUF,
# https://github.com/Tribler/software-based-PUF
# Modifications and additions Copyright (C) 2023, 2024 myndcryme
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
from PUF import SerialPUF, Tools

# temporarily append puf_xtra package if not already in sys.path
pkg = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'puf_xtra'))
if Tools.sys_path_append(pkg) != 0:
    print("error appending path")
    sys.exit(1)
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
        if is_sram_23lc1024 is None:
            raise ValueError("SRAM module config invalid, check setting")
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

if __name__ == "__main__":      # guard so we can import functions without running the whole script
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
            sdx = bindutils.get_sram_index(device=s)
            if not sdx.startswith("ERR"):       # sram match
                print("match found\nsram index: %s\nstable bits location file already exists, so it is "
                      "not necessary to run GetStableBits.py\nYou can generate a new strongbits file if desired "
                      "but is not recommended.\n" % sdx)
                res = portutils.get_user_response("Generate a new strongbits location file? [y/N]\n> ")
                if res is True:
                    thread = StableBitsGenerator(thread_name='thread0', is_sram_23lc1024=bindutils.is_sram_val(),
                                                 serialconnection=s, bitrate=conf.BITRATE, index=sdx)
                    thread.start()
                    thread.join()       # wait
                elif res is False:
                    print("exiting...")
                    sys.exit(0)
                else:
                    sys.exit(2)     # error: invalid response
            elif sdx == "ERR_NO_VALUES_INDEX":     # no existing index in 'Values' directory
                print("no existing sram index was found\nrunning gen_sram_profile...")
                sdx = bindutils.gen_sram_profile(device=s)       # generates profile and returns index
                if not sdx.startswith("ERR"):       # current sram location and values files creation succeeded
                    print("new sram profile generated successfully\nstrongbits location file and values file for "
                          "sram '%s' have been created... exit" % sdx)
                    sys.exit(0)
                else:       # error
                    print("error: %s occurred while trying to create sram profile... exit" % sdx)
                    sys.exit(3)
            elif sdx == "ERR_NO_VALUES_FILE":
                print("error: %s, exit" % sdx)      # TODO: handle properly
                sys.exit(7)
            elif sdx == "ERR_DISTANCE_CALCULATION":
                print("error: %s, exit" % sdx)
                sys.exit(4)
            elif sdx == "ERR_INDEX_NOT_ASSIGNED":       # not fatal - means no sram index found
                print("no sram match found\ngenerating new sram profile...")
                sdx = bindutils.gen_sram_profile(device=s)      # create profile for current sram
                if not sdx.startswith("ERR"):
                    print("New sram profile '%s' created successfully... exit" % sdx)
                    sys.exit(0)
                else:
                    print("error: %s occurred trying to create sram profile... exit" % sdx)
                    sys.exit(5)
        else:
            print('device not found... exit')
            sys.exit(1)

    # operation mode is set in conf.py
    elif conf.OPERATION_MODE == 'PARALLEL' and conf.USE_BINDING:
        # **************** parallelly generate stable bits ****************
        # must set devices in the configuration file via SERIAL_DEVICE param (use device paths)
        # creates a thread for each device given in configuration list
        # each device unique id is bound to an idx value and the idx value is set accordingly
        # for each thread(device) so the user no longer needs to set manually

        # check to make sure we can get all unique_id and idx values without error
        n = len(conf.SERIAL_DEVICE)             # number of serial devices
        unique_id = ['' for _ in range(n)]      # empty n item list
        idx = ['' for _ in range(n)]
        err=0

        # added GET_UID case to Arduino code for cy62256nll and 23lc1024
        for i in range(n):
            unique_id[i] = bindutils.get_uid(conf.SERIAL_DEVICE[i])     # gets unique_id
            if unique_id[i] == '':      # break on error
                err += 1
                break
            idx[i] = bindutils.get_index(conf.BIND_FILE, unique_id=unique_id[i])    # now get the index (or create)
            if idx[i] == '':        # error, increment error counter and break
                err += 1
                break
        if err > 0:
            print('error getting device unique id and/or idx binding\n'
                  'unable to proceed, exit')
            sys.exit(1)

        # all serial devices are properly bound, all should succeed generating stable bits
        threads=[]
        for i in range(len(conf.SERIAL_DEVICE)):
            tname = 'thread' + str(i)
            thread = StableBitsGenerator(thread_name=tname, is_sram_23lc1024=bindutils.is_sram_val(),
                                         serialconnection=conf.SERIAL_DEVICE[i], bitrate=conf.BITRATE, index=idx[i])
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
        thread1 = StableBitsGenerator(thread_name='thread1', is_sram_23lc1024=bindutils.is_sram_val(),
                                      serialconnection='/dev/cu.usbmodem14111', bitrate=115200, index='A')
        thread1.start()
        thread2 = StableBitsGenerator(thread_name='thread2', is_sram_23lc1024=bindutils.is_sram_val(),
                                      serialconnection='/dev/cu.usbmodem14121', bitrate=115200, index='C')
        thread2.start()

# **************** solo generating stable bits ****************
# thread1 = stableBitsGenerator(is_sram_23lc1024=False, serialconnection='/dev/cu.usbmodem14111', bitrate=115200,
#                               idx='A')
# thread1.start()

# begin ARCHIVE_C0
#     idx = ''
#         unique_id = bindutils.get_uid(s)
#         if unique_id:
#             idx = bindutils.get_index(conf.BIND_FILE,unique_id=unique_id)
#         else:
#             print('get_uid error\n')
#             sys.exit(2)
#         if idx:
# end
