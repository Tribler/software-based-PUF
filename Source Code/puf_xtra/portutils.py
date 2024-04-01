# This file enables device detection and is part of software-based-PUF,
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
import os
import shutil
import sys
import serial
import serial.tools.list_ports
import importlib
from conf import conf, const        # double import
from puf_xtra import user

def save_param_setting(name, val) -> int:
    """
    Sets user specified value for a configuration parameter.
    :param name: config parameter name for which to set the value
    :param val: config parameter value to set
    :return: 0 on success, 1 on error
    """
    filename = conf.USER_FILE

    try:
        with open(filename, 'r') as f:
            line_list = f.readlines()
    except Exception as e:
        if isinstance(e, PermissionError):
            print('error: permission denied')
        else:
            print('error: ', e)
        return 1

    for i, line in enumerate(line_list):
        if name in line:
            index = line.find(name)     # position of first char of name matched in line
            if isinstance(val, str):
                line_list[i] = line[:index+len(name)] + " = '" + val + "'\n"        # enclose string in quotes
            else:
                line_list[i] = line[:index+len(name)] + " = " + val + "\n"          # other types, don't enclose

    with open(filename, 'w') as f:
        f.writelines(line_list)
    importlib.reload(user)              # reload after parameter value change
    return 0

def get_user_response(msg: str):
    """
    Display msg, read and return the user response.
    :msg: display message
    :return: True (yes), False (no), None on error
    """
    y = {'y','Y','yes','Yes','YES', 'yES'}      # set of acceptable yes answers
    n = {'n','N','no','No','NO', 'nO', ""}      # set of acceptable no answers, plus *just* <enter>

    result = None
    ver = sys.version_info.major
    if ver>2:  # use input function
        try:
            result = input(msg)
        except ValueError:
            print('error: invalid entry')
            return None
    elif ver<=2:  # use raw_input
        try:
            result = raw_input(msg)
        except ValueError:
            print('error: invalid entry')
            return None
    else:
        return None
    if result in y:
        return True
    elif result in n:
        return False
    else:
        print("invalid entry")
        return None

def get_user_input(msg):        # TODO: redundant but no harm, fix later
    """
    Prompt user and read input.
    :msg: display message
    :return: user selected value, -1 on error
    """
    result = ''
    ver=sys.version_info.major
    if ver>2:  # use input function
        try:
            result = input(msg)
        except ValueError:
            print('error: invalid entry')
            return -1
    elif ver<=2:  # use raw_input
        try:
            result = raw_input(msg)
        except ValueError:
            print('error: invalid entry')
            return -1
    return result

def selected_device_reset() -> int:
    """
    Resets user selected device information.  Selection parameters saved
    in user.py will be reset to defaults.
    :return: 0 on success, 1 on error
    """
    src = 'puf_xtra/.user_restore.py'
    dest = 'puf_xtra/user.py'

    try:
        shutil.copy(src, dest)
        os.chmod(dest, 0o644)
        importlib.reload(user)      # reload user module after reset
        return 0
    except Exception as e:
        if isinstance(e, PermissionError):
            print('error: permission denied')
        else:
            print('error: ', e)
        return 1

def selected_device_keep() -> bool:
    """
    Displays current user selected device information and prompts
    the user for confirmation to either keep using the selected device
    or de-select the device so another may be chosen.
    :return: True (use selected) or False (de-select, use other)
    """
    print("\nCurrent user selected device: ")
    print('manufacturer: '+str(user.selected_manufacturer)+'\n\tproduct: '+\
          str(user.selected_product)+'\n\thwid: '+str(user.selected_hwid)+'\n')

    y = {'y', 'Y', 'yes', 'Yes', 'YES'}     # set of acceptable yes answers
    n = {'n', 'N', 'no', 'No', 'NO'}        # set of acceptable no answers
    msg = 'Use this device (y/n)?\n> '
    result = get_user_input(msg)

    if result in y:
        return True
    elif result in n:
        return False
    else:
        print("invalid entry")
        return False

def detect() -> str:
    """
    Detects if specified device (default is genuine MEGA2560_R3) is
    connected and acquires device info, otherwise displays a list of
    serial devices to choose from manually.
    :return: device, empty string if no device, or RERUN
    """
    s = const.EMPTY     # path string, initialized to empty string
    ports = list(serial.tools.list_ports.comports())
    if conf.SERIAL_DEVICE.upper() != const.DEFAULT_DEVICE_ID:        # user specified a device in configuration file
        for p in ports:
            if conf.SERIAL_DEVICE.upper() in p.hwid:        # user specified a device VID:PID
                s = str(p.device)
                print('device id detected...\nusing device ' + s)
        for p in ports:
            if conf.SERIAL_DEVICE in p.device:      # user specified a device
                s = str(p.device)
                print('device found\nusing device ' + s)

    elif user.selected_vidpid != const.EMPTY:       # user previously selected a device, VID:PID was saved
        for p in ports:
            if user.selected_vidpid in p.hwid:      # VID:PID found in list of current serial devices
                result = selected_device_keep()     # use previous selected device (y/n), boolean result
                if result:      # yes
                    s = str(p.device)                   # associated path of VID:PID
                    print('using device ' + s)
                else:
                    selected_device_reset()         # de-select previously selected device
                    return const.RERUN              # return and re-run

    elif user.selected_device != const.EMPTY:       # user previously selected a device, path was saved
        for p in ports:
            if user.selected_device in p.device:    # device path found in list
                result = selected_device_keep()
                if result:      #yes
                    s = str(p.device)
                    print('using device ' + s)
                else:
                    selected_device_reset()         # de-selected device
                    return const.RERUN              # re-run detect()

    elif conf.SERIAL_DEVICE.upper() == const.DEFAULT_DEVICE_ID:         # detect genuine Arduino board
        for p in ports:
            if conf.SERIAL_DEVICE.upper() in p.hwid:        # VID:PID found
                s = str(p.device)                           # associated path of VID:PID
                print('Arduino MEGA2560_R3 detected...\nusing device ' + s)

    # TODO: elif: add MEGA2560_R3 clone VID:PID info for port auto-detection

    if s == const.EMPTY:     # not detected
        print('\nno device detected...\ncheck if device is unplugged\n'      \
              'if using a generic board you may select from the following list\n\nserial device list:')
        n=0
        for p in ports:
            n+=1
            print('(' + str(n) + ')\t', end='')     # no newline ending
            print('manufacturer: ' + str(p.manufacturer) + '\n\tproduct: ' + \
                  str(p.product) + '\n\thwid: ' + str(p.hwid) + '\n')
        n=0  # input value
        ver = sys.version_info.major
        if ver > 2:  # use input function
            try:
                n = int(input('\nselect device number or 0 to exit\n> '))
            except ValueError:
                print('error: invalid entry')
        elif ver == 2:  # use raw_input
            try:
                n = int(raw_input('\nselect device number or 0 to exit\n> '))
            except ValueError:
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
                save_param_setting('selected_device', s)
                if p.vid is not None and p.pid is not None:
                    vidpid = str('%04x' % p.vid) + ':' + str('%04x' % p.pid)    # convert vid,pid int values to string
                    save_param_setting('selected_vidpid', vidpid.upper())
                if p.manufacturer is not None and p.product is not None and p.hwid is not None:
                    save_param_setting('selected_manufacturer', p.manufacturer)
                    save_param_setting('selected_product', p.product)
                    save_param_setting('selected_hwid', p.hwid)
    return s

if conf.DEBUG_FILE:     # for isolated debugging
    result = const.RUN
    while result == const.RUN:
        result = detect()
    sys.exit(0)