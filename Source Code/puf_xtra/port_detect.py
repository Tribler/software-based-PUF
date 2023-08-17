# Adds easy device path detection to software-based-PUF,
# https://github.com/Tribler/software-based-PUF
# Copyright (C) 2023 myndcryme
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

# from __future__ import print_function       # for python2
import sys
import serial
import serial.tools.list_ports
import puf_consts as pc


def detect(device_id=pc.device.GENUINE_MEGA2560_R3):
    """
    Detects if specified device (default is genuine MEGA2560_R3) is
    connected and acquires device info, otherwise displays a list of
    serial devices to choose from manually.

    :param device_id: VID:PID of device or user defined device path str
    :return: device path or empty string if device not found
    """
    s = ''  # path string
    ports = list(serial.tools.list_ports.comports())
    if device_id.upper() == pc.device.GENUINE_MEGA2560_R3:      # detect genuine Arduino board
        for p in ports:
            if device_id.upper() in p.hwid:
                s = str(p.device)  # device path
                print('Arduino MEGA2560_R3 detected...\nusing device ' + s)
    #elif: add MEGA2560_R3 clone VID:PID info here for port auto-detection
    else:  # detect user specified/generic board or user spec. path
        for p in ports:
            if device_id.upper() in p.hwid:  # user specified board VID:PID
                s = str(p.device)
                print('device detected...\nusing device ' + s)
        for p in ports:
            if device_id in p.device:  # user specified path
                s = str(p.device)
                print('user specified path found\nusing device ' + s)

    if s == '':  # not detected
        print('device not detected...\ncheck if device is unplugged\n'      \
              'or you may be using a generic board\n\nserial device list:')

        n=0
        for p in ports:
            n+=1
            print('(' + str(n) + ')\t', end='')
            print('manufacturer: ' + str(p.manufacturer) + '\n\tproduct: ' + \
                  str(p.product) + '\n\thwid: ' + str(p.hwid) + '\n')

        n=0  # input value
        ver = sys.version_info.major
        if ver > 2:  # use input function
            try:
                n = int(input('\nselect device number or 0 to exit\n> '))
            except ValueError:
                n=0
                print('error: invalid entry')
        elif ver == 2:  # use raw_input
            try:
                n = int(raw_input('\nselect device number or 0 to exit\n> '))
            except ValueError:
                n=0
                print('error: invalid entry')
        else:
            print('\npython version unsupported\n')

        if n == 0:
            return s  # exit, return empty string

        i=0
        for p in ports:
            i+=1
            if i == n:
                s = str(p.device)
    return s
