import serial, time, os, errno
import datetime, math
from struct import *
from PUF.Result import *
from PUF.Tools import *
import numpy
# from decimal import *

class SerialPUF:
    ser = serial.Serial
    folder = "fff"
    is_turn_on = False
    has_reply = True
    is_sram_23lc1024 = True

    def connect(self, name, port, is_sram_23lc1024=True):
        try:
            self.ser = serial.Serial(name, port)
            self.is_sram_23lc1024 = is_sram_23lc1024
            now = datetime.datetime.now()
            self.folder = str(now.year) + str(now.month) + str(now.day)
            return True
        except serial.SerialException:
            return False


    def collect_pages(self):
        current_page = 0
        if self.is_sram_23lc1024:
            total_page = 4096
        else:
            total_page = 1024
        bits = []

        while current_page < total_page:
            self.read_page(current_page, bits)
            current_page = current_page + 1

        return bits

    def collect_bytes(self):
        current_byte = 0
        total_byte = 131072
        if not self.is_sram_23lc1024:
            total_byte = 0x8000
        bits = []

        while current_byte < total_byte:
            a = self.read_byte(current_byte)
            # bits.append(a.value)
            bits.append(bin(a.value)[2:].zfill(8))
            current_byte = current_byte + 1

        return bits

    def collect_data(self, beginIndex, endIndex, delay=10):
        # give delay to ensure arduino is ready
        time.sleep(delay)

        currentIndex = beginIndex
        while (currentIndex <= endIndex):
            print("--------------- ITERATION " + str(currentIndex) + " -------------------")
            self.turn_on_sram()

            # if self.is_sram_23lc1024:
            bits = self.collect_pages()
            # else:
            #     bits = self.collect_bytes()
            save_to_file(bits, "../CY62256NLL/" + create_today_date() + "/" + str(currentIndex))
            # save_to_file(bits, "../2018130/" + str(currentIndex))
            # self.saveToFile(bits, currentIndex)

            self.turn_off_sram()
            time.sleep(delay)

            currentIndex = currentIndex + 1

    def get_all_data(self, delay=10):
        self.turn_off_sram(is_delay=True)
        time.sleep(delay)

        self.turn_on_sram(is_delay=True)

        bits = self.collect_pages()
        return bits

    def get_data(self, delay=10, address=[]):
        values = []
        self.turn_on_sram(is_delay=True)
        for k in address:
            a = self.read_bit(int(k))
            values.append(a.value)

        self.turn_off_sram()
        time.sleep(delay)

        return values

    def turn_on_sram(self, is_delay=False):
        while not self.is_turn_on:
            self.send_command_turn_on(is_delay=is_delay)
            res = self.check_serial_data()
            self.process_command(res)

    def turn_off_sram(self, is_delay=False):
        while self.is_turn_on:
            self.send_command_turn_off(is_delay=is_delay)
            # print("turn off please")
            res = self.check_serial_data()
            self.process_command(res)

    def check_serial_data(self):
        res = bytearray(b'')
        count = 0
        while count < 36:
            if (self.ser.inWaiting() > 0):
                kon = self.ser.read()
                res.append(kon[0])
                count = count + 1
        return res

    def send_command_turn_on(self, is_delay=False):
        # print("write voltage " + str(analog_voltage))
        val = 1 if is_delay else 0
        self.ser.write(pack('<bbBbh', 99, 41, val, 0, 0))

    def send_command_turn_off(self, is_delay=False):
        val = 1 if is_delay else 0
        # self.ser.write(pack('<bbl', 99, 40, 0))
        self.ser.write(pack('<bbBbh', 99, 40, val, 0, 0))

    def send_command_read_page(self, page):
        # print("read page " + str(page))
        self.ser.write(pack('<bbhh', 99, 42, page, 0))
        self.has_reply = False

    def send_command_read_byte(self, address):
        # print("read single " + str(address))
        if self.is_sram_23lc1024:
            self.ser.write(pack('<bbl', 99, 43, address))
        else:
            self.ser.write(pack('<bbHH', 99, 43, address, 0))
        self.has_reply = False

    def send_command_read_bit(self, address):
        self.ser.write(pack('<bbl', 99, 49, address))
        self.has_reply = False

    def send_command_write_one(self):
        self.ser.write(pack('<bbl', 99, 44, 0))
        self.has_reply = False

    def send_command_write_zero(self):
        self.ser.write(pack('<bbl', 99, 45, 0))
        self.has_reply = False

    def send_command_write_page(self, page, is_one):
        self.ser.write(pack('<bbHbb', 99, 47, page, is_one, 0))
        self.has_reply = False

    def send_command_write_byte(self, address, data=0xFF):
        if self.is_sram_23lc1024:
            self.ser.write(pack('<bbL', 99, 48, address))
        else:
            self.ser.write(pack('<bbHH', 99, 48, address, data))
        self.has_reply = False

    def send_command_append_challenges(self, address):
        self.ser.write(pack('<bbL', 99, 51, address))
        self.has_reply = False

    def send_command_delete_challenges(self):
        self.ser.write(pack('<bbL', 99, 50, 0))
        self.has_reply = False

    def send_command_finish_writing_challenges(self):
        self.ser.write(pack('<bbL', 99, 52, 0))
        self.has_reply = False

    def send_command_generate_helper_data(self):
        self.ser.write(pack('<bbL', 99, 53, 0))
        self.has_reply = False

    def send_command_get_keys(self):
        self.ser.write(pack('<bbL', 99, 54, 0))
        self.has_reply = False

    def process_command(self, res, bits=[]):
        # print(res)
        num_of_bits = 8
        if (res[1] == 40):
            self.is_turn_on = False
            r = unpack('>bbH', res[0:4])
            # print("TURNED OFF " + str(r[2] * 5 / 1023))
        elif (res[1] == 41):
            self.is_turn_on = True
            r = unpack('>bbH', res[0:4])
            print("TURNED ON " + str(r[2]) + " : " + str(r[2] * 5 / 1023))
        elif (res[1] == 42):
            rp = ReadPageResult(res)
            for a in rp.bytes:
                bits.append(bin(a)[2:].zfill(num_of_bits))
            self.has_reply = True
            return rp
        elif (res[1] == 43):
            rb = ReadByteResult(res, is_sram_23lc1024=self.is_sram_23lc1024)
            self.has_reply = True
            return rb
        elif (res[1] == 44):
            print("WRITE ONES")
            # self.has_reply = True
            rp = WritePageResult(res)
            for a in rp.bytes:
                bits.append(bin(a)[2:].zfill(num_of_bits))
            self.has_reply = True
            return rp
        elif (res[1] == 45):
            print("WRITE ZEROS")
            rp = WritePageResult(res)
            for a in rp.bytes:
                bits.append(bin(a)[2:].zfill(num_of_bits))
            self.has_reply = True
            return rp
        elif (res[1] == 46):
            self.is_turn_on = True
            r = unpack('>bbBH', res[:5])
            print("TURNED ON " + str(r[2]) + " " + str(r[3]))
        elif (res[1] == 47):
            rp = WritePageResult(res)
            for a in rp.bytes:
                bits.append(bin(a)[2:].zfill(num_of_bits))
            self.has_reply = True
            return rp
        elif (res[1] == 48):
            r = WriteByteResult(res, is_sram_23lc1024=self.is_sram_23lc1024)
            self.has_reply = True
            return r
        elif (res[1] == 49):
            r = ReadBitResult(res)
            self.has_reply = True
            return r
        elif (res[1] == 50):
            print("REMOVE CHALLENGES")
            self.has_reply = True
        elif (res[1] == 51):
            print("APPEND CHALLENGES")
            self.has_reply = True
        elif (res[1] == 52):
            print("FINISH APPEND CHALLENGES")
            self.has_reply = True
        elif (res[1] == 53):
            print("GENERATE HELPER DATA")
            self.has_reply = True
        elif (res[1] == 54):
            print("GET KEYS")
            self.has_reply = True
            return res[4:]
        else:
            print("UNKNOWN COMMAND " + str(res[1]))

    def read_page(self, page, bits):
        self.send_command_read_page(page)
        res = self.check_serial_data()
        # print(str(page) + " - " + str(res))
        return self.process_command(res, bits)

    def read_byte(self, byt):
        ret = ReadByteResult
        while ret.location != byt:
            self.send_command_read_byte(byt)
            res = self.check_serial_data()
            ret = self.process_command(res)
            if isinstance(ret, ReadByteResult):
                if ret.location == byt:
                    return ret

    def read_bit(self, bit):
        # byt = self.read_byte(int(bit / 8))
        # a = bin(byt.value)[2:].zfill(8)
        # return a[bit % 8]
        ret = ReadBitResult
        while ret.location != bit:
            self.send_command_read_bit(bit)
            res = self.check_serial_data()
            ret = self.process_command(res)
            if isinstance(ret, ReadBitResult):
                if ret.location == bit:
                    return ret

    def collect_stable_bits(self, bits, delay=2, goal=2200, iteration=1000, save=True, write=False):
        start = str(datetime.datetime.now().time())
        bit_values = {}
        self.turn_off_sram()
        time.sleep(delay)

        # initialize
        self.turn_on_sram()
        asd = []
        for i in bits:
            r = self.read_bit(int(i))
            bit_values[i] = r
            asd.append(str(i) + ", " + str(r))
        self.turn_off_sram()
        time.sleep(delay)

        # if save:
            # save_to_file(bit_values, "../result/" + start)
            # save_to_file(asd, "../result/asd")

        # start
        j = 0
        # while len(bit_values) > goal and j < iteration:
        while j < iteration:
            print("--------------- ITERATION " + str(j) + " : " + str(len(bit_values)) + "-------------------")
            self.turn_on_sram(analog_voltage=130 + (j%125))
            bit_to_remove = []
            for i in bit_values:
                if not i:
                    continue
                r = self.read_bit(int(i))

                if r != bit_values[i]:
                    bit_to_remove.append(i)
            for i in bit_to_remove:
                bit_values.pop(i)
            if write:
                if j % 2 == 0:
                    self.send_command_write_zero()
                else:
                    self.send_command_write_one()
            self.turn_off_sram()
            time.sleep(delay)

            if save:
                save_to_file(bit_values, "../result/20180129-4")
            #     save_to_file(asd, "../result/asd" + start)

            # for i in asd:
            #     print(i)
            j += 1

        print(len(bit_values))

        return bit_values

    def get_bits_with_value(self, bits, delay = 2):
        bit_values = {}
        self.turn_off_sram()
        time.sleep(delay)

        # initialize
        self.turn_on_sram()
        # asd = []
        for i in bits:
            r = self.read_bit(int(i))
            bit_values[i] = r
            # asd.append(str(i) + ", " + str(r))
        self.turn_off_sram()
        time.sleep(delay)

        return bit_values

    def collect_stable_bits_with_initial(self, bits_value, delay=2, goal=2200, iteration=1000, save=True, write=False):
        self.turn_off_sram()
        time.sleep(delay)

        # start
        j = 0
        while j < iteration:
            print("--------------- ITERATION " + str(j) + " : " + str(len(bits_value)) + "-------------------")
            self.turn_on_sram()

            bit_to_remove = []
            for i in bits_value:
                if not i:
                    continue
                r = self.read_bit(int(i))

                if r != bits_value[i]:
                    bit_to_remove.append(i)
            for i in bit_to_remove:
                bits_value.pop(i)

            # if write:
            #     if j % 2 == 0:
            #         self.send_command_write_zero()
            #     else:
            #         self.send_command_write_one()
            self.turn_off_sram()
            time.sleep(delay)

            if save:
                save_to_file(bits_value, "../result/asd12")
            #     save_to_file(asd, "../result/asd" + start)

            # for i in asd:
            #     print(i)
            j += 1

        # print(len(bit_values))

        return bits_value

    def try_data_remanence(self, delay, write_ones):
        self.turn_on_sram()
        self.write_pages(write_ones)
        self.turn_off_sram()

        time.sleep(delay)
        self.turn_on_sram()

        ones = 0
        zeroes = 0
        strong_bits = []
        bits = self.collect_pages()
        pos = 0
        for i in bits:
            for j in i:
                if int(j) == 0:
                    zeroes += 1
                    if write_ones:
                        strong_bits.append(pos)
                else:
                    ones += 1
                    if not write_ones:
                        strong_bits.append(pos)
                pos += 1
        self.turn_off_sram()

        return [delay, zeroes, ones, strong_bits]

    def collect_bits_using_data_remanence(self, init, write_ones, goal=2500):
        delay = init

        strong_bits_count = 0
        while strong_bits_count < goal:
            pos = 0
            ones = 0
            zeroes = 0
            strong_bits = []

            self.turn_on_sram()
            self.write_pages(write_ones)
            self.turn_off_sram()

            time.sleep(delay)
            self.turn_on_sram()

            bits = self.collect_pages()

            for i in bits:
                for j in i:
                    if int(j) == 0:
                        zeroes += 1
                        if write_ones:
                            strong_bits.append(pos)
                    else:
                        ones += 1
                        if not write_ones:
                            strong_bits.append(pos)
                    pos += 1
            self.turn_off_sram()

            delay += 0.01

            # time.sleep(5)

            if write_ones:
                strong_bits_count = zeroes
            else:
                strong_bits_count = ones

            print(str(delay) + ", " + str(zeroes) + ", " + str(ones) + ", " + str(strong_bits_count))

        return [delay, zeroes, ones, strong_bits]

    def write_page(self, page, is_one, bits):
        self.send_command_write_page(page, is_one)
        res = self.check_serial_data()
        return self.process_command(res, bits)

    def write_byte(self, byt, data=0xFF):
        self.send_command_write_byte(byt, data=data)
        res = self.check_serial_data()
        return self.process_command(res)

    def write_pages(self, is_ones):
        bits = []
        current_page = 0
        if self.is_sram_23lc1024:
            total_page = 4096
        else:
            total_page = 1024

        while current_page < total_page:
            self.write_page(current_page, is_ones, bits)
            current_page = current_page + 1

    def open_challenges_on_sd(self):
        self.send_command_delete_challenges()
        res = self.check_serial_data()
        self.process_command(res)

    def append_challenges_on_sd(self, address):
        self.send_command_append_challenges(address)
        res = self.check_serial_data()
        self.process_command(res)

    def close_challenges_on_sd(self):
        self.send_command_finish_writing_challenges()
        res = self.check_serial_data()
        self.process_command(res)

    def analyze_stable_bits_on_time(self, bits, initial=0.5, iteration=20, save=True, filename=""):
        if save and not filename:
            print("Filename should not be empty when save is true")
            return
        delay = initial
        bit_values = {}
        stable = []
        self.turn_off_sram()
        time.sleep(5)
        bits_length = len(bits)

        # initialize
        self.turn_on_sram()
        asd = []
        for i in bits:
            r = self.read_bit(int(i))
            bit_values[i] = r
            asd.append(str(i) + ", " + str(r))

        # start
        j = 0
        unstable = 0
        while j < iteration:
            print("--------------- ITERATION " + str(j) + " : " + str(delay) + "-------------------")
            self.turn_off_sram()
            time.sleep(delay)
            self.turn_on_sram()

            for i in bit_values:
                if not i:
                    continue
                r = self.read_bit(int(i))

                if r != bit_values[i]:
                    unstable += 1

            stable.append([delay, bits_length-unstable])

            delay = 2 * delay
            unstable = 0

            if save:
                save_to_file(stable, filename)

            j += 1

        return stable

    def analyze_stable_bits_on_voltage(self, bits, delay=10, initial_voltage=4.5, end_voltage=5,
                                       iteration=20, save=True, filename=""):
        if save and not filename:
            print("Filename should not be empty when save is true")
            return
        bit_values = {}
        stable = []
        self.turn_off_sram()
        time.sleep(delay)
        bits_length = len(bits)

        # initialize
        self.turn_on_sram(analog_voltage=255)
        asd = []
        for i in bits:
            r = self.read_bit(int(i))
            bit_values[i] = r
            asd.append(str(i) + ", " + str(r))

        # start
        unstable = 0

        for voltage in numpy.arange(4.5, 5.1, 0.1):
            print("--------------- VOLTAGE : " + str(voltage) + "-------------------")
            for j in range(iteration):
                print(j)
                self.turn_off_sram()
                time.sleep(delay)
                self.turn_on_sram(analog_voltage=voltage / 5 * 255)

                for i in bit_values:
                    if not i:
                        continue
                    r = self.read_bit(int(i))

                    if r != bit_values[i]:
                        unstable += 1

                stable.append([voltage, bits_length-unstable])

                unstable = 0

                if save:
                    save_to_file(stable, filename)

        return stable

    def write_challenges_to_sd(self, challenges):
        self.open_challenges_on_sd()
        for i in challenges:
            # print(i)
            self.append_challenges_on_sd(int(i))
        self.close_challenges_on_sd()

    def generate_helper_data_on_sd(self):
        self.send_command_generate_helper_data()
        res = self.check_serial_data()
        self.process_command(res)

    def get_keys(self):
        self.send_command_get_keys()
        res = self.check_serial_data()
        return self.process_command(res)

    def check_if_it_is_working(self, delay=5):
        self.turn_on_sram(is_delay=True)
        time.sleep(0.5)

        data = 0xFF
        count_wrong = 0
        for i in range(0, 1024):

            bits = []
            a = self.write_page(int(i), True, bits)
            # print(a.bytes)
            # bits = []
            b = self.read_page(int(i), bits)
            # print(b.bytes)
            for j in b.bytes:
                # print(j)
                if j != data:
                    count_wrong += 1

        self.turn_off_sram(is_delay=True)
        time.sleep(delay)

        print(count_wrong)
        if count_wrong > 0:
            print("error")
            exit()
        else:
            print("working fine")