import os
import sys
import itertools
from PUF import Tools, Analyzer     #, DataRemanenceCalculator
from time import sleep

# temporarily append puf_xtra package if not already in sys.path
pkg = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'puf_xtra')
Tools.sys_path_append(pkg)
import bindutils
import portutils
from conf import const, conf

h = Analyzer.Analyzer()

def read_folder(folder):
    result = []
    for f in os.listdir(folder + os.sep):      # use os.sep instead of hardcoding "/"
        file = folder + os.sep + f
        if os.path.isdir(file):
            result.extend(read_folder(file))
        else:
            if not file.endswith(".DS_Store"):
                result.append(read_file(file))
    return result


def read_file(file):
    return Tools.read_bits_from_file_and_merge(file)


def calculate_stability(index_SRAM):
    folder = 'Values' + os.sep + index_SRAM
    files = read_folder(folder)
    return h.calculate_intra_hamming_distance_between_elements(files, length=4662)

if conf.OPERATION_MODE == 'MONO':
    s = const.RUN
    while s == const.RUN:
        s = portutils.detect()
    if s:
        index = ''
        unique_id = bindutils.get_uid(s)
        if unique_id:
            index = bindutils.get_index_nc(conf.BIND_FILE, unique_id=unique_id)
        else:
            print("get_uid failed")
            sys.exit(2)
        if index:
            result = calculate_stability(index_SRAM=index)
            print("Average Pct : " + str(result[0]))
            print("Highest Pct : " + str(result[3]))
            print("Lowest Pct \t: " + str(result[4]))
        else:
            print("get_index failed")
            sys.exit(3)
    else:
        print("device not found... exit")
        sys.exit(1)

elif conf.OPERATION_MODE == 'PARALLEL': # added parallel operation code
    n = len(conf.SERIAL_DEVICE)         # number of serial devices
    unique_id = ['' for _ in range(n)]  # empty n item list
    index = ['' for _ in range(n)]      # empty n item list
    err=0

    for i in range(n):
        unique_id[i] = bindutils.get_uid(conf.SERIAL_DEVICE[i])  # get unique_id
        if unique_id[i] == '':  # break on error
            err+=1
            break
        index[i] = bindutils.get_index_nc(conf.BIND_FILE, unique_id=unique_id[i])  # get the idx, no create
        if index[i] == '':  # error, increment error counter and break
            err+=1
            break
    if err>0:
        print('error getting device unique id and/or idx binding\nunable to proceed, exit')
        sys.exit(1)
    # all idx values found, proceed
    for i in range(n):
        result = calculate_stability(index_SRAM=index[i])
        print("\n##### Results for SERIAL_DEVICE[" + str(i) + "] #####")
        print("Average Pct : " + str(result[0]))
        print("Highest Pct : " + str(result[3]))
        print("Lowest Pct \t: " + str(result[4]))
        sleep(0.5)
    sys.exit(0)
else:
    print("error: no operation mode set in config.py")
    sys.exit(4)
