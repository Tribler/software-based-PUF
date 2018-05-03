import numpy
import time

from PUF import SerialPUF, Tools
import threading


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


# thread1 = StableBitsValueGetter(is_sram_23lc1024=False, serialconnection='/dev/cu.usbmodem14111',
#                                 bitrate=115200, index='A', bits_filename='strongbits-A.txt',
#                                 saved_filename='3')
# thread1.start()

thread2 = StableBitsValueGetter(is_sram_23lc1024=False, serialconnection='/dev/cu.usbmodem14121',
                                bitrate=115200, index='B', bits_filename='strongbits-A.txt',
                                saved_filename='1')
thread2.start()
