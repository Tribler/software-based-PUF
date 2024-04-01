import os
import time

def get_sram_index(device, dep):
    """
    Checks current SRAM against known SRAM results.  Parses directories containing
    strongbits location files and associated strongbits values files.  Iterates through
    any existing files, comparing the current SRAM results.  SRAM matches an indexed
    SRAM if dissimilar cell value count is < error threshold (ERR_THRESH)*4662.

    :param device: device path of the current arduino
    :param dep: module dependency (injection), SerialPUF in this case
    :return: index value bound to sram, empty on error
    """
    bound = False
    index = ''

    # dependency injection, #GetStableBits.py #MainEnrollment.py
    # dependency = __import__('SerialPUF.py')
    # bindutils.get_sram_index(device='/dev/ttyACM0', dep=dependency)


    # mod_path = "/home/user/PycharmProjects/myndcryme/software-based-PUF/Source Code/puf_xtra/bindutils.py"
    # src_path = os.path.normpath(os.path.join(mod_path, "..", ".."))     # traverse up two directories ('Source Code')
    src_path = "/home/user/git/software-based-PUF/Source Code/"
    loc_path = os.path.join(src_path, "enrollment and testing/master-testing")      # strongbits location files
    val_path = os.path.join(loc_path, "Values")

    idx=[]
    fc_date=[]      # first created date directory list
    val_list=[]
    fc_val=[]       # first created value file
    try:
        idx = [n for n in os.listdir(val_path) if os.path.isdir(os.path.join(val_path,n))]     # {index}
    except FileNotFoundError as e:
        print("no sram index entries found")
        return e
    for i,n in enumerate(idx):
        p = os.path.join(val_path, n)       # /val_path/{index}
        sub = [j for j in os.listdir(p) if os.path.isdir(os.path.join(p, j))]
        c = time.time()             # init to current time
        print("index: ", n, ", sub dir: ", sub)
        for k in sub:
            q = os.path.join(p, k)      # /val_path/{index}/{date}
            if os.path.getctime(q) < c:
                c = os.path.getctime(q)
                if len(fc_date) > i:
                    fc_date[i] = q
                else:
                    fc_date.insert(i, q)       # first created date dir for index 'n'
        print("first created date list: ", fc_date[i])
        try:
            val_list = [f for f in os.listdir(fc_date[i]) if os.path.isfile(os.path.join(fc_date[i], f))]   # {file}
        except FileNotFoundError as e:
            print("no strongbits value file found")
            return e
        print("value list: ", val_list)
        c = time.time()  # re-init
        for a,b in enumerate(val_list):
            p = os.path.join(fc_date[i], b)
            if os.path.getctime(p) < c:
                c = os.path.getctime(p)
                if len(fc_val) > i:
                    fc_val[i] = p
                else:
                    fc_val.insert(i, p)
        print("value file: ", fc_val[i])
    return index

get_sram_index()