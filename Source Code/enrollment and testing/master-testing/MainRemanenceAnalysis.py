import time

from PUF import SerialPUF, Tools
import numpy


serialPUF = SerialPUF.SerialPUF()
if not serialPUF.connect('/dev/cu.usbmodem1411', 115200, is_sram_23lc1024=False):
    print("Error connecting to Arduino")
    exit(1)
time.sleep(2)

# -------------------------------------------------------------------------------------------------------
# ---------------------------------------- CHECK IF IT'S WORKING -----------------------------------
# -------------------------------------------------------------------------------------------------------

serialPUF.check_if_it_is_working()

# -------------------------------------------------------------------------------------------------------
# --------------------------------------- DATA REMANENCE ------------------------------------------
# -------------------------------------------------------------------------------------------------------
# ------------------------------ COLLECT 1 & 0 COUNTS -------------------------------------
r = []
is_write_one = False
filename = "../CY62256NLL/remanences-count/A/remanence0/" + Tools.create_today_date()

for i in numpy.arange(0.2, 0.3, 0.01):
    print("DATA REMANENCE - delay: " + str(i))
    result = serialPUF.try_data_remanence(i, is_write_one)
    r.append(str(result[0]) + " - " + str(result[1]) + ", " + str(result[2]))
    print(str(result[0]) + " - " + str(result[1]) + ", " + str(result[2]))
    # Tools.save_to_file(r, filename)

r = []
is_write_one = True
filename = "../CY62256NLL/remanences-count/A/remanence1/" + Tools.create_today_date()

for i in numpy.arange(0.2, 0.3, 0.01):
    print("DATA REMANENCE - delay: " + str(i))
    result = serialPUF.try_data_remanence(i, is_write_one)
    r.append(str(result[0]) + " - " + str(result[1]) + ", " + str(result[2]))
    print(str(result[0]) + " - " + str(result[1]) + ", " + str(result[2]))
    # Tools.save_to_file(r, filename)

# ------------------------------ COLLECT STRONG BITS -------------------------------------
r = []
i = 0.28
is_write_one = True
filename = "../CY62256NLL/remanences-location/A/remanence1/bits-" + str(i) + "-" + Tools.create_today_date()

print("COLLECT BITS USING DATA REMANENCE - delay: " + str(i))
result = serialPUF.try_data_remanence(i, is_write_one)
print(str(result[0]) + " - " + str(result[1]) + ", " + str(result[2]))
Tools.save_to_file(result[3], filename)

r = []
i = 0.28
is_write_one = False
filename = "../CY62256NLL/remanences-location/A/remanence0/bits-" + str(i) + "-" + Tools.create_today_date()

print("COLLECT BITS USING DATA REMANENCE - delay: " + str(i))
result = serialPUF.try_data_remanence(i, is_write_one)
print(str(result[0]) + " - " + str(result[1]) + ", " + str(result[2]))
Tools.save_to_file(result[3], filename)
