# This file is part of software-based-PUF.
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

import os
import serial
import time
from struct import pack
from conf import conf, const

def bind_file_chk(filename: str=conf.BIND_FILE):        # bindings.csv check
    """
    Checks for existence of BIND_FILE and creates an empty file if not found.
    :return: none
    """
    mod_path = os.path.abspath(__file__)        # bindutils.py path
    pkg_path = os.path.dirname(mod_path)        # puf_xtra pkg path
    csv_path = os.path.abspath(os.path.join(pkg_path, filename))
    if not os.path.isfile(csv_path):
        with open(filename,'w') as f:     # create file
            f.write("")

def get_index(filename: str=conf.BIND_FILE, unique_id: str='') -> str:
    """
    Locates unique_id bind entry in csv file if already bound and returns the index
    associated with the uid.  If not bound, an entry will be created and the index
    value returned.
    :param filename: csv file for bind entries
    :param unique_id: arduino unique identifier
    :return: index bound to unique_id, empty on error
    """
    bound = False
    index = const.EMPTY     # init empty str
    sep =','
    try:
        with open(filename, 'r') as f:
            line_list = f.readlines()
    except Exception as e:
        print('error: ', e)
        return const.EMPTY

    for line in line_list:
        if unique_id in line and line.endswith('\n'):       # uid already bound (with line ending chk)
            index = line.strip().split(sep)[0]              # index value
            bound = True
            break

    if not bound:
        if unique_id == const.EMPTY:        # no uid data
            return const.EMPTY              # return empty
        if not line_list:       # empty file, start index at 'A'
            line = 'A' + sep + unique_id + '\n'
            index = 'A'
        else:                   # file not empty
            last_line = line_list[len(line_list)-1]         # last line
            last_index = last_line.strip().split(sep)[0]    # last index value (character)
            next_index = chr(ord(last_index) + 1)           # next index
            if not last_line.endswith('\n'):                # incorrect line ending
                with open(filename, 'a') as f:
                    f.write('\n')                           # append newline
            line = next_index + sep + unique_id + '\n'      # make sure to add newline
            index = next_index
        try:
            with open(filename, 'a') as f:
                f.write(line)                   # append bind entry
        except Exception as e:
            if isinstance(e, PermissionError):
                print('error: permission denied')
            else:
                print('error: ', e)
            return const.EMPTY
    return index

def get_uid_index(filename: str=conf.BIND_FILE, uid: str='') -> str:    # don't need, use get_index()
    """
    Returns index value bound to uid.
    :param filename: csv file for bind entries (index,uid)
    :param uid: device unique_id (hex string)
    :return: index bound to given uid, empty on error
    """
    sep = ','
    index = const.EMPTY
    try:
        with open(filename, 'r') as f:
            line_list = f.readlines()
    except Exception as e:
        print('error: ', e)
        return index

    for line in line_list:
        if uid in line and line.endswith('\n'):     # uid found and line has appropriate ending
            index = line.strip().split(sep)[0]      # strip whitespace and split - index is first token
    return index

def get_uid(serial_device_path: str = '') -> str:
    """
    Writes GET_UID (55) command to Arduino serial device port.  Arduino device
    utilizes the ArduinoUniqueID library to retrieve the unique id and returns
    the uid hex string.
    :param serial_device_path: Arduino device path
    :return: unique id for given device, empty on error
    """
    try:
        ser = serial.Serial(serial_device_path, baudrate=conf.BITRATE, timeout=5)      # start serial connection
        time.sleep(1)
        ser.write(pack('<bbL', 99, const.GET_UID, 0))                   # send GET_UID command
        uid = ser.read(const.UID_BUF_SIZE).decode('utf-8').strip()      # read and decode response
        print('uid: ' + uid)
        ser.close()
    except Exception as e:
        print('error: ', e)
        return const.EMPTY
    return uid

bind_file_chk(filename=conf.BIND_FILE)     # check for bindings file existence, create if necessary
