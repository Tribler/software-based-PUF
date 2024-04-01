# This file is part of software-based-PUF.
# https://github.com/Tribler/software-based-PUF
# Copyright (C) 2023, 2024 myndcryme
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
import sys
import shutil
import time
import tempfile
#from datetime import datetime
from typing import Optional
from struct import pack
from scipy.spatial import distance
from conf import conf, const
import wrapper

path0 = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'enrollment and testing', 'master-enrollment')
path1 = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'enrollment and testing', 'master-testing')
paths = [path0, path1]
wrapper.sys_path_append(paths)

def hamming_distance(file1, file2) -> Optional[float]:
    """
    Calculates hamming distance between two sram stable bits value results.
    :param file1: stable bit values file (1-D array)
    :param file2: bit values of current sram (1-D array)
    :return: hamming distance, None on error
    """
    dist=None
    with open(file1, 'r') as a, open(file2, 'r') as b:
        data1 = [line.strip() for line in a]
        data2 = [line.strip() for line in b]
    if len(data1) != len(data2):
        print("len(data1): %i, len(data2): %i" % (len(data1), len(data2)))
        print("error: arrays are not equal length")
    else:
        dist = distance.hamming(data1, data2)
    return dist

def is_sram_val() -> Optional[bool]:
    """
    Checks config file SRAM chip type and returns corresponding bool value.
    :return: True if sram is 23lc1024, False if sram is cy62256nll, None on error.
    """
    if conf.SRAM == 'CY62256NLL':
        return False
    elif conf.SRAM == '23LC1024':
        return True
    else:
        return None     # error condition, invalid sram config

def bind_file_chk(filename: str=conf.BIND_FILE):
    """
    Checks for existence of file and creates an empty file in the puf_xtra
    package directory with 'filename' if not found.
    :param filename: file checked for existence
    :return: None
    """
    mod_path = os.path.abspath(__file__)        # bindutils.py
    pkg_path = os.path.dirname(mod_path)        # puf_xtra pkg
    csv_path = os.path.abspath(os.path.join(pkg_path, filename))
    if not os.path.isfile(csv_path):
        with open(filename,'w') as f:     # create file
            f.write("")

# ***** See notes about CDF of binomial distribution for matching rational ***** #
# ***** CDF (cumulative distribution function) ********************************* #
def get_sram_index(device: str) -> str:
    """
    Checks current SRAM against known SRAM results.  Parses directories containing
    strongbits location files and associated strongbits values files.  Iterates through
    any existing indexed entries, comparing the current SRAM results.  SRAM matches an
    indexed SRAM if dissimilar cell value count is < error threshold (ERR_THRESH)*4662.
    :param device: connected arduino device path
    :return: sram index value on success, else an error code (listed)
    :raises ERR_INDEX_NOT_ASSIGNED: no sram profile was found
    :raises ERR_NO_VALUES_INDEX: no subdirectories found in 'Values" folder
    :raises ERR_NO_VALUES_FILE: no appropriate values file found
    :raises ERR_DISTANCE_CALCULATION: hamming calculation error occurred
    """
    def delete_orphaned_profile(path: str, ndx: str) -> bool:
        """
        Deletes entire sram profile (location and values).
        :param path: '.../Values/{index}' branch to remove
        :param ndx: {index} string used to remove strongbits file
        :return: True on success, False on error
        """
        if os.path.exists(path):
            if path == val_path or path == val_path+os.sep:     # sanity check
                return False        # don't accidentally delete the whole Values index
            file = "strongbits-" + ndx + ".txt"
            filepath = os.path.join(loc_path, file)
            try:
                shutil.rmtree(path)
                os.remove(filepath)
                return True
            except PermissionError:
                print("permission error encountered trying to remove orphaned index")
                return False
            except OSError:
                print("oserror occurred while trying to remove orphaned index")
                return False
        else:
            return False

    index = "ERR_INDEX_NOT_ASSIGNED"
    mod_path = os.path.dirname(os.path.abspath(__file__))
    src_path = os.path.normpath(os.path.join(mod_path, ".."))       # traverse up to directory 'Source Code'
    loc_path = os.path.join(src_path, "enrollment and testing", "master-testing")      # strongbits location files
    val_path = os.path.join(loc_path, "Values")

    # only need to compare against one strongbits (location,values) set
    # using last created files for comparison was not chosen arbitrarily, it is necessary
    idx=[]
    orphans=[]
    val_list=[]
    try:
        idx = [n for n in os.listdir(val_path) if os.path.isdir(os.path.join(val_path,n))]     # {index}
    except FileNotFoundError:       # misleading - raised for directories not found here
        return "ERR_NO_VALUES_INDEX"     # no index subdirectories in val_path
    lc_date = [None]*len(idx)       # list for last created date directory
    lc_val = [None]*len(idx)        # list for last created values file
    for i,n in enumerate(idx):
        p = os.path.join(val_path, n)       # /val_path/{index}
        sub = [j for j in os.listdir(p) if os.path.isdir(os.path.join(p, j))]
        t = 0.0             # reference time, init to epoch
        for k in sub:
            q = os.path.join(p, k)      # /val_path/{index}/{date}
            if os.path.getctime(q) > t:
                t = os.path.getctime(q)
                lc_date[i] = q      # last created date directory for index n
        try:
            val_list = [f for f in os.listdir(lc_date[i]) if os.path.isfile(os.path.join(lc_date[i], f))]   # {file}
        except TypeError:       # TypeError raised if lc_date[i] is None
            # indicates no date directory was found, thus no values files for index n, can delete orphaned profile
            orphans.append([p,n])       # delete orphans later
            continue        # short circuit and try remaining
        t = 0.0         # re-init
        for a,b in enumerate(val_list):
            p = os.path.join(lc_date[i], b)
            if os.path.getctime(p) > t:
                t = os.path.getctime(p)
                lc_val[i] = p            # last created values file for each sram index, {i}

    from GetStableBitsValue import StableBitsValueGetter    # deferred function import to avoid circular logic
    for i,n in enumerate(idx):
        idx_is_orphan = any(n == o[1] for o in orphans)
        if idx_is_orphan: continue       # skips comparison below for this iteration

        print("checking if sram '%s' is a match..." % n)
        tname = "sram" + str(i)
        bfname = "strongbits-" + n + ".txt"
        tmp = tempfile.NamedTemporaryFile(mode='w+t', delete=True)
        # stable bits values written to tempfile for the current sram at index {n} strongbits location
        thread = StableBitsValueGetter(thread_name=tname, is_sram_23lc1024=is_sram_val(), serialconnection=device,
                              bitrate=conf.BITRATE, index=n, bits_filename=bfname, saved_filename='', tmp=tmp)
        thread.start()
        thread.join()
        # calculate hamming distance of prior indexed sram and current sram (for comparison)
        dist = hamming_distance(lc_val[i], tmp.name)     # dist is the proportion of disagreeing cells
        if dist is not None and dist <= conf.ERR_THRESH:
            index = n       # match
            print(dist, "<=", conf.ERR_THRESH)
            break
        elif dist is not None and dist > conf.ERR_THRESH:
            print(dist, ">", conf.ERR_THRESH, " -> sram does not match")
        else:
            index = "ERR_DISTANCE_CALCULATION"
            tmp.close() if not tmp.closed else None
            break
        tmp.close() if not tmp.closed else None
    # we can now safely delete any orphaned profiles
    for o in orphans:
        delete_orphaned_profile(o)
    return index

def get_index_list(path: str):
    """
    Gets existing index values from strongbits location files.  Only returns *valid* indices
    (auto-generated).
    :param path: where the strongbits-{index}.txt files are located
    :return: list of existing {index} or None on error
    """
    prefix="strongbits-"
    suffix=".txt"
    def extract_valid_index(filename: str) -> Optional[str]:
        """
        Extracts the index from a strongbits location filename.  Only filenames using
        single character index are valid by convention (avoids non-auto assigned index).
        :param filename: the strongbits location file (i.e. strongbits-A.txt)
        :return: index on success or None on error
        """
        try:
            index = filename.split(prefix,1)[1].split(suffix,1)[0]
            try:
                res = ord(index)    # checks if single char
            except TypeError:
                return None         # discards any invalid index
        except IndexError:
            return None
        return index
    index_list = []
    try:
        file_list = [f for f in os.listdir(path) if prefix in f]
    except FileNotFoundError:
        print("exception: FileNotFoundError")
        print("path: %s" % path)
        exit(1)     # TODO: fix
        return None
    for f in file_list:
        if extract_valid_index(f) is not None:
            index_list.append(extract_valid_index(f))
    return sorted(index_list)

def gen_sram_profile(device: str) -> str:
    """
    Generates a profile (strongbits location and values files) for the current connected sram,
    used for sram indexing/binding.
    :param device: device path for connected arduino
    :return: index binding on successful profile creation, errcode on fail
    """
    def remove_orphaned_tree(path) -> bool:
        if os.path.exists(path):
            if path == val_path or path == val_path+os.sep:
                return False        # don't bork the whole 'Values' directory by mistake
            try:
                shutil.rmtree(path)
                return True
            except PermissionError:
                print("permission error encountered trying to remove tree")
                return False
            except OSError:
                print("oserror occurred while trying to remove orphaned tree")
                return False
        else:
            return False

    index="ERR_INDEX_NOT_ASSIGNED"
    mod_path = os.path.dirname(os.path.abspath(__file__))
    src_path = os.path.normpath(os.path.join(mod_path,".."))        # traverse up to directory 'Source Code'
    loc_path = os.path.join(src_path,"enrollment and testing","master-testing")  # strongbits location files
    val_path = os.path.join(loc_path,"Values")

    # determine the existing indexes and assign new profile to next index but fill any incomplete
    # entries (holes) in the index list - also if a strongbits location file has no matching values file,
    # overwrite it with a new profile

    index_list = get_index_list(loc_path)       # list of existing index extracted from strongbits-{index}.txt
    if index_list:
        idx = [ord(ch) for ch in index_list]
        idx_range = set(range(65, max(idx)+1))      # ord('A') == 65 (where indexing begins)
        m = sorted(idx_range - set(idx))            # gets any missing index (as ordinal number)
        if m:
            index = chr(m.pop(0))       # sets the to be generated sram profile to missing index
            p = os.path.join(val_path, index)
            remove_orphaned_tree(p)     # checks for orphaned tree in 'Values' directory and deletes if present
        else:
            index = chr(max(idx)+1)
    elif not index_list:            # empty, start at 'A'
        index = 'A'
    else:                   # None indicates an error
        return "ERR_UNDEFINED"

    from GetStableBits import StableBitsGenerator           # deferred import
    thread = StableBitsGenerator(thread_name='bits-gen', is_sram_23lc1024=is_sram_val(), serialconnection=device,
                                 bitrate=conf.BITRATE, index=index)
    thread.start()
    thread.join()       # wait
    print("bits location file created")
    time.sleep(1.0)
    print("getting bits values...")
    from GetStableBitsValue import StableBitsValueGetter
    bfname = "strongbits-" + index + ".txt"
    suffix = '1'        # we are creating the first Values file
    thread = StableBitsValueGetter(thread_name='bits-val', is_sram_23lc1024=is_sram_val(), serialconnection=device,
                                   bitrate=conf.BITRATE, index=index, bits_filename=bfname, saved_filename=suffix)
    thread.start()
    thread.join()       # wait to finish
    print("bits values file created")
    time.sleep(1.0)
    return index

def get_index(filename: str=conf.BIND_FILE, unique_id: str='') -> str:
    """
    Locates unique_id bind entry in csv file if already bound and returns the idx
    associated with the uid.  If not bound, an entry will be created and the idx
    value returned.
    :param filename: csv file for bind entries
    :param unique_id: arduino unique identifier
    :return: idx bound to unique_id, empty on error
    """
    bound = False
    index = ''
    sep =','
    try:
        with open(filename, 'r') as f:
            line_list = f.readlines()
    except Exception as e:
        print('error: ', e)
        return const.EMPTY

    for line in line_list:
        if unique_id in line and line.endswith('\n'):       # uid already bound (with line ending chk)
            index = line.strip().split(sep)[0]              # idx value
            bound = True
            break

    if not bound:
        if unique_id == const.EMPTY:        # no uid data
            return const.EMPTY              # return empty
        if not line_list:       # empty file, start idx at 'A'
            line = 'A' + sep + unique_id + '\n'
            index = 'A'
        else:                   # file not empty
            last_line = line_list[len(line_list)-1]         # last line
            last_index = last_line.strip().split(sep)[0]    # last idx value (character)
            next_index = chr(ord(last_index) + 1)           # next idx
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

def get_index_nc(filename: str=conf.BIND_FILE, unique_id: str='') -> str:       # get_index_nc(), no create idx value
    """
    Returns idx value bound to the unique_id.  Does not create an idx value
    if the value does not exist.
    :param filename: csv file for bind entries (idx,uid)
    :param unique_id: device unique_id (hex string)
    :return: idx bound to given uid, empty on error
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
        if unique_id in line and line.endswith('\n'):       # uid found and line has appropriate ending
            index = line.strip().split(sep)[0]              # strip whitespace and split - idx is first token
    return index

def get_uid(serial_device_path: str = '') -> str:
    """
    Writes GET_UID (55) command to Arduino serial device port.  Arduino device
    utilizes the ArduinoUniqueID library to retrieve the unique id and returns
    the uid hex string.
    :param serial_device_path: Arduino device f
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

bind_file_chk(filename=conf.BIND_FILE)      # check for bindings file existence, create if necessary
