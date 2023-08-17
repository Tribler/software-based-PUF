import numpy
import time

from PUF import SerialPUF, Tools

# set true if testing is done on microchip 23lc1024
is_sram_23lc1024 = False

# TODO: add device auto detect?
serialPUF = SerialPUF.SerialPUF()
if not serialPUF.connect('/dev/cu.usbmodem1411', 115200, is_sram_23lc1024):
    print("Error connecting to Arduino")
    exit(1)
time.sleep(2)

# -------------------------------------------------------------------------------------------------------
# ---------------------------------------- CHECK IF IT'S WORKING -----------------------------------
# -------------------------------------------------------------------------------------------------------
serialPUF.check_if_it_is_working()
#
# -------------------------------------------------------------------------------------------------------
# ---------------------------- COLLECT VALUES FROM ALL LOCATIONS INSIDE SRAM ----------------------------
# -------------------------------------------------------------------------------------------------------
# for j in numpy.arange(2.5, 4.5, 0.5):
#     for i in range(3):
#         print("--------------- ITERATION " + str(i) + " -------------------")
#         bits = serialPUF.get_all_data(delay=5)
#         Tools.save_to_file(bits, "../23LC1024/all-locations/E/2018-04-11/" + str(j) + "/" + str(i))

# -------------------------------------------------------------------------------------------------------
# ---------------------------------------- GET VALUE OF STRONG BITS -----------------------------------
# -------------------------------------------------------------------------------------------------------

# ****************************************** 23LC1024 **********************************************
# ---------------------------------------- REMANENCE -----------------------------------
# filename = "../23LC1024/remanences-2018-02-22"
# bits = Tools.read_bits_from_file(filename, is_separated_by_comma=True)
# values = serialPUF.get_data(delay=5,address=bits)
# Tools.save_to_file(values, "../23LC1024/stable-remanences/remanences-2018-02-22/voltage-variation/5/" + Tools.create_today_date())
#
# # ---------------------------------------- NEIGHBOR -----------------------------------
# filename = "../23LC1024/bitsfiltered-2018-02-19"
# bits = Tools.read_bits_from_file(filename)
# values = serialPUF.get_data(delay=10, address=bits)
# Tools.save_to_file(values, "../23LC1024/stable-neighbor/bitsfiltered-2018-02-19/voltage-variation/5/" + Tools.create_today_date())

# ****************************************** CY62256NLL **********************************************
# ---------------------------------------- REMANENCE -----------------------------------
# filename = "../CY62256NLL/remanences-location/A/remanences-0.28-2018-02-15"
# # filename = "../CY62256NLL/remanences-location/E/E-remanences-0.28-2018-03-27"
# bits = Tools.read_bits_from_file(filename, is_separated_by_comma=True)
# print(len(bits))
# values = serialPUF.get_data(delay=5,address=bits)
# Tools.save_to_file(values, "../CY62256NLL/stable-remanences/A-remanences-0.28-2018-02-15/voltage-variation/4.5/" + Tools.create_today_date())
#
# # ---------------------------------------- NEIGHBOR -----------------------------------
# filename = "../CY62256NLL/neighbor-highrank-location/A/bitsfiltered-2018-04-05"
# bits = Tools.read_bits_from_file(filename)
# values = serialPUF.get_data(delay=10, address=bits)
# Tools.save_to_file(values, "../CY62256NLL/stable-neighbor/A-bitsfiltered-2018-04-05/voltage-variation/4.0/" + Tools.create_today_date())