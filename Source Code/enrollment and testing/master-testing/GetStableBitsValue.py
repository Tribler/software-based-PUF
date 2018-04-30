import numpy
import time

from PUF import SerialPUF, Tools

# set true if testing is done on microchip 23lc1024
is_sram_23lc1024 = False

serialPUF = SerialPUF.SerialPUF()
if not serialPUF.connect('/dev/cu.usbmodem1411', 115200, is_sram_23lc1024):
    print("Error connecting to Arduino")
    exit(1)
time.sleep(2)

# serialPUF.check_if_it_is_working()

filename = "strongbits.txt"
bits = Tools.read_bits_from_file(filename, is_separated_by_comma=True)
values = serialPUF.get_data(delay=5, address=bits)

index_SRAM = 'A'
saved_file_name = "Values/" + index_SRAM + "/" + Tools.create_today_date() + "/4"
Tools.save_to_file(values, saved_file_name)
