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
import glob
import errno
import datetime
import time
import threading
from PUF import SerialPUF, Tools

# Append puf_xtra package path.  New modules of reused code are organized in the puf_xtra package.
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'puf_xtra'))   # add puf_xtra path
import portutils
import bindutils
from conf import conf, const

def get_next_suffix(index: str='') -> str:
    """
    Parses filenames from the directory used by Tools.save_to_file() for the device
    specified by the index value and determines the next saved file name suffix.
    Creates the sub path if not found and begins suffix naming at '1'.
    :param index: the index value bound to the device uid
    :return: suffix for naming the next file, empty on error
    """
    if index == '':     # can't get suffix without an index value
        return ''
    base_path = os.path.dirname(os.path.abspath(__file__))
    sub_path = 'Values/' + index + '/' + str(datetime.date.today()) + '/'       # saved_file_name path
    full_path = os.path.join(base_path, sub_path)
    if not os.path.exists(full_path) and len(full_path) > 0:
        try:
            os.makedirs(full_path)
        except OSError as e:        # guard against race condition
            if e.errno != errno.EEXIST:
                raise

    files = glob.glob(full_path + '*')
    suffixes = []
    for f in files:
        # Values/{index}/YYYY-MM-DD/{suffix}
        suffix = os.path.basename(f)    # basename('/foo/bar/suffix') returns suffix, but '/foo/bar/' returns ''
        suffixes.append(suffix)

    if suffixes:
        next_suffix = str(max(suffixes)+1)
    else:
        next_suffix = '1'       # begin suffix at '1'

    return next_suffix

class StableBitsValueGetter(threading.Thread):
    def __init__(self, thread_name, is_sram_23lc1024, serialconnection, bitrate, index, bits_filename, saved_filename):
        threading.Thread.__init__(self, name=thread_name)
        self.is_sram_23lc1024 = is_sram_23lc1024
        self.serialconnection = serialconnection
        self.bitrate = bitrate
        self.index = index
        self.filename = bits_filename
        self.saved_filename = saved_filename

    def run(self):
        # set true if testing is done on microchip 23lc1024
        serialPUF = SerialPUF.SerialPUF()
        if not serialPUF.connect(self.serialconnection, self.bitrate, self.is_sram_23lc1024):
            print("Error connecting to Arduino")
            exit(1)
        time.sleep(2)

        # serialPUF.check_if_it_is_working()

        bits = Tools.read_bits_from_file(self.filename, is_separated_by_comma=True)
        values = serialPUF.get_data(delay=5, address=bits)

        saved_file_name = "Values/" + self.index + "/" + Tools.create_today_date() + "/" + self.saved_filename
        Tools.save_to_file(values, saved_file_name)

# TODO: add class parameter indicating voltage_variation or time_interval mode usage
# TODO: add Arduino code to measure pin voltage and use the value in file naming
# TODO: possibly add support for a buck boost with programmable voltage output such as
# TODO: TPS6381xEVM for auto voltage variation testing and auto file naming

# operation mode is set in configuration file, conf.py
if conf.OPERATION_MODE == 'MONO':
    # **************** generate bits, single serial device with auto detection ****************
    # detect() will automatically set 'serialconnection' param when possible
    # user may set a device manually via SERIAL_DEVICE in conf.py but should usually not be necessary
    # detection progression:  configuration file --> previous selected device --> MEGA2560_R3 (arduino) -->
    # MEGA2560_R3 (clone) --> display serial devices (user select and save)
    err=0
    s = const.RUN
    while s == const.RUN:       # runs again if detect() returns RERUN          (RERUN == RUN)
        s = portutils.detect()
    if s:       # s not empty
        mono_idx = ''
        mono_unique_id = bindutils.get_uid(s)
        if mono_unique_id:
            mono_idx = bindutils.get_index(conf.BIND_FILE, unique_id=mono_unique_id)
        else:
            err+=1
        if mono_idx:
            bfname = 'strongbits-' + str(mono_idx) + '.txt'     # bits_filename
            suffix = get_next_suffix(mono_idx)
            thread = StableBitsValueGetter(thread_name='thread0', is_sram_23lc1024=False, serialconnection=s,
                                           bitrate=conf.BITRATE, index=mono_idx, bits_filename=bfname,
                                           saved_filename=suffix)
            thread.start()
        else:
            err+=1
        if err>0:
            print('error getting device uid and/or index... exit\n')
            sys.exit(2)
    else:
        print('device not found... exit')
        sys.exit(1)

# operation mode is set in conf.py
elif conf.OPERATION_MODE == 'PARALLEL' and conf.USE_BINDING:                # use updated parallel code
    # check to make sure we can get all unique_id and index values without error
    n = len(conf.SERIAL_DEVICE)  # number of serial devices
    unique_id = ['' for _ in range(n)]  # empty n item list
    idx = ['' for _ in range(n)]  # empty n item list
    err=0

    # added GET_UID case to Arduino code for cy62256nll and 23lc1024
    for i in range(n):
        unique_id[i] = bindutils.get_uid(conf.SERIAL_DEVICE[i])  # gets unique_id
        if unique_id[i] == '':  # break on error
            err+=1
            break
        idx[i]=bindutils.get_index(conf.BIND_FILE, unique_id=unique_id[i])  # now get the index (created if absent)
        if idx[i] == '':  # error, increment error counter and break
            err+=1
            break
    if err>0:
        print('error getting device unique id and/or index binding\n'
              'unable to proceed, exit')
        sys.exit(1)

    # all serial devices are properly bound, all should succeed
    threads=[]
    for i in range(len(conf.SERIAL_DEVICE)):
        # TODO: test bfname and suffix additions
        bfname = 'strongbits-' + str(idx[i]) + '.txt'      # bits_filename
        suffix = get_next_suffix(idx[i])      # saved_filename
        tname = 'thread' + str(i)
        thread= StableBitsValueGetter(thread_name=tname, is_sram_23lc1024=False, serialconnection=conf.SERIAL_DEVICE[i],
                                      bitrate=conf.BITRATE, index=idx[i], bits_filename=bfname,
                                      saved_filename=suffix)
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()       # wait until all threads finish

    # threads will execute concurrently but all code hereafter will be blocked until threads finish
    print('all threads finished')
    sys.exit(0)

# operation mode is set in conf.py
elif conf.OPERATION_MODE == 'PARALLEL' and conf.USE_LEGACY:

    # user needs to set values manually and update bits_filename, saved_filename as specified in docs
    thread1 = StableBitsValueGetter(thread_name='thread1', is_sram_23lc1024=False,
                                    serialconnection='/dev/cu.usbmodem14111', bitrate=115200, index='A',
                                    bits_filename='strongbits-A.txt', saved_filename='3')
    thread1.start()

    thread2 = StableBitsValueGetter(thread_name='thread2', is_sram_23lc1024=False,
                                    serialconnection='/dev/cu.usbmodem14121', bitrate=115200, index='B',
                                    bits_filename='strongbits-A.txt', saved_filename='1')
    thread2.start()
