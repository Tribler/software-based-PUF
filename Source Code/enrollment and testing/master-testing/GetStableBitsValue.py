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
import numpy
import time
import threading
from PUF import SerialPUF, Tools

# Append puf_xtra package path.  New modules of reused code are organized in the puf_xtra package.
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'puf_xtra'))   # add puf_xtra path
import port_detect as pd
import puf_consts as pc


class StableBitsValueGetter(threading.Thread):
    def __init__(self, is_sram_23lc1024, serialconnection, bitrate, index, bits_filename, saved_filename):
        threading.Thread.__init__(self)
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


# ***** mode of operation *****
mode = pc.mode.DEFAULT          # default mode is a single Arduino board
# mode = pc.mode.PARALLEL       # set this mode for parallel profiling

if mode == pc.mode.DEFAULT:
    # **************** single serial device with auto detection ****************
    # device auto detected or may be selected by user
    # serialconnection param may be set manually w/ device path or VID:PID (but not required)
    #
    # examples :     s = pd.detect()     s = pd.detect('/dev/ttyACM0')     s = pd.detect('2341:0042')

    s = pd.detect()  # may be set manually by the user but is not necessary (see examples above)
    if s != '':
        print('using device path ' + s)
        thread2=StableBitsValueGetter(is_sram_23lc1024=False,serialconnection=s,bitrate=115200,index='B',
                                      bits_filename='strongbits-A.txt',saved_filename='1')
        thread2.start()
    else:
        print('device not found... exit')
        sys.exit()
    # ***** open an issue and provide your working Arduino clone VID:PID to add for auto detection *****

elif mode == pc.mode.PARALLEL:
    # TODO: add auto detection for parallel profiling
    # ***** use this if profiling multiple boards simultaneously *****
    thread1 = StableBitsValueGetter(is_sram_23lc1024=False, serialconnection='/dev/cu.usbmodem14111',
                                    bitrate=115200, index='A', bits_filename='strongbits-A.txt', saved_filename='3')
    thread1.start()

    thread2 = StableBitsValueGetter(is_sram_23lc1024=False, serialconnection='/dev/cu.usbmodem14121',
                                    bitrate=115200, index='B', bits_filename='strongbits-A.txt', saved_filename='1')
    thread2.start()
