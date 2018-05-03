from PUF.Result import *


class SerialPUF:
    ser = serial.Serial
    is_turn_on = False
    has_reply = True

    def connect(self, name, port):
        """
        Connect to an Arduino
        :param name: location of Arduino
        :param port: port to connect to Arduino
        :return: True if successfully connect to Arduino
        """
        try:
            self.ser = serial.Serial(name, port)
            return True
        except serial.SerialException:
            return False

    def collect_pages(self):
        """
        Collect all pages values in Cypress CY62256NLL
        :return:
        """
        current_page = 0
        total_page = 1024
        bits = []

        while current_page < total_page:
            self.read_page(current_page, bits)
            current_page = current_page + 1

        return bits

    def get_all_data(self, delay=10):
        """
        Get all bits in all location
        :param delay:
        :return:
        """
        self.turn_off_sram(is_delay=True)
        time.sleep(delay)

        self.turn_on_sram(is_delay=True)

        bits = self.collect_pages()
        return bits

    def get_data(self, delay=10, address=[]):
        """
        Get bit values given the bit addresses
        :param delay:
        :param address:
        :return:
        """
        values = []
        self.turn_on_sram(is_delay=True)
        for k in address:
            a = self.read_bit(int(k))
            values.append(a.value)

        self.turn_off_sram()
        time.sleep(delay)

        return values

    def turn_on_sram(self, is_delay=False):
        """
        Turn on SRAM Cypress CY62256NLL
        :param is_delay: set to true if want to wait 1s after turning on the SRAM
        """
        while not self.is_turn_on:
            self.send_command_turn_on(is_delay=is_delay)
            res = self.check_serial_data()
            self.process_command(res)

    def turn_off_sram(self, is_delay=False):
        """
        Turn off SRAM Cypress CY62256NLL
        :param is_delay: set to true if want to wait 1s after turning off the SRAM
        """
        while self.is_turn_on:
            self.send_command_turn_off(is_delay=is_delay)
            res = self.check_serial_data()
            self.process_command(res)

    def check_serial_data(self):
        """
        Checking incoming data through serial until there are 36 bytes data
        :return:
        """
        res = bytearray(b'')
        count = 0
        while count < 36:
            if (self.ser.inWaiting() > 0):
                kon = self.ser.read()
                res.append(kon[0])
                count = count + 1
        return res

    def send_command_turn_on(self, is_delay=False):
        """
        Send command to Arduino to turn on the SRAM Cypress CY62256NLL
        :param is_delay:
        """
        val = 1 if is_delay else 0
        self.ser.write(pack('<bbBbh', 99, 41, val, 0, 0))

    def send_command_turn_off(self, is_delay=False):
        """
        Send command to Arduino to turn off the SRAM Cypress CY62256NLL
        :param is_delay:
        """
        val = 1 if is_delay else 0
        self.ser.write(pack('<bbBbh', 99, 40, val, 0, 0))

    def send_command_read_page(self, page):
        """
        Read a page in SRAM Cypress CY62256NLL. A page contains 32 bytes values
        :param page: page address
        """
        self.ser.write(pack('<bbhh', 99, 42, page, 0))
        self.has_reply = False

    def send_command_read_byte(self, address):
        """
        Read a byte in SRAM Cypress CY62256NLL.
        :param address: byte address
        """
        self.ser.write(pack('<bbHH', 99, 43, address, 0))
        self.has_reply = False

    def send_command_read_bit(self, address):
        """
        Read a bit in SRAM Cypress CY62256NLL.
        :param address: bit address
        """
        self.ser.write(pack('<bbl', 99, 49, address))
        self.has_reply = False

    def send_command_write_page(self, page, is_one):
        """
        Write a page in SRAM Cypress CY62256NLL
        :param page: page address
        :param is_one: set to true if want to write one to all location in the page
        """
        self.ser.write(pack('<bbHbb', 99, 47, page, is_one, 0))
        self.has_reply = False

    def send_command_write_byte(self, address, data=0xFF):
        """
        Write a byte in SRAM Cypress CY62256NLL
        :param address: byte address
        :param data: data to be written
        """
        self.ser.write(pack('<bbHH', 99, 48, address, data))
        self.has_reply = False

    def send_command_append_challenges(self, address):
        """
        Send an address as part of PUF challenge to Arduino. Arduino will append the challenge to microSD
        :param address:
        """
        self.ser.write(pack('<bbL', 99, 51, address))
        self.has_reply = False

    def send_command_new_challenges(self):
        """
        Send command to Arduino to add new challenges to microSD. Arduino will initialize the writing challenges
        process to microSD
        """
        self.ser.write(pack('<bbL', 99, 50, 0))
        self.has_reply = False

    def send_command_finish_writing_challenges(self):
        """
        Send command to Arduino to notify that the challenges is completely written. Arduino will close the challenge
        file in microSD
        """
        self.ser.write(pack('<bbL', 99, 52, 0))
        self.has_reply = False

    def send_command_generate_helper_data(self):
        """
        Ask the Arduino to generate the helper data. The challenge has to be available in microSD
        """
        self.ser.write(pack('<bbL', 99, 53, 0))
        self.has_reply = False

    def send_command_get_keys(self):
        """
        Get PUF generated keys. The challenge has to be available in microSD
        """
        self.ser.write(pack('<bbL', 99, 54, 0))
        self.has_reply = False

    def process_command(self, res, bits=[]):
        """
        Process incoming data through the Serial port
        :param res:
        :param bits:
        :return:
        """
        num_of_bits = 8
        if res[1] == 40:
            self.is_turn_on = False
            r = unpack('>bbH', res[0:4])
            # print("TURNED OFF " + str(r[2]) + " : " + str(r[2] * 5 / 1023))
        elif res[1] == 41:
            self.is_turn_on = True
            r = unpack('>bbH', res[0:4])
            # print("TURNED ON  " + str(r[2]) + " : " + str(r[2] * 5 / 1023))
        elif res[1] == 42:
            rp = ReadPageResult(res)
            for a in rp.bytes:
                bits.append(bin(a)[2:].zfill(num_of_bits))
            self.has_reply = True
            return rp
        elif res[1] == 43:
            rb = ReadByteResult(res)
            self.has_reply = True
            return rb
        elif res[1] == 46:
            self.is_turn_on = True
            r = unpack('>bbBH', res[:5])
            # print("TURNED ON " + str(r[2]) + " " + str(r[3]))
        elif res[1] == 47:
            rp = WritePageResult(res)
            for a in rp.bytes:
                bits.append(bin(a)[2:].zfill(num_of_bits))
            self.has_reply = True
            return rp
        elif res[1] == 48:
            r = WriteByteResult(res)
            self.has_reply = True
            return r
        elif res[1] == 49:
            r = ReadBitResult(res)
            self.has_reply = True
            return r
        elif res[1] == 50:
            print("NEW CHALLENGES")
            return res[2]
            self.has_reply = True
        elif res[1] == 51:
            # print("APPEND CHALLENGES")
            self.has_reply = True
        elif res[1] == 52:
            print("FINISH APPEND CHALLENGES")
            self.has_reply = True
        elif res[1] == 53:
            print("GENERATE HELPER DATA")
            self.has_reply = True
        elif (res[1] == 54):
            print("GET KEYS")
            self.has_reply = True
            return res[4:]
        else:
            print("UNKNOWN COMMAND " + str(res[1]))

    def read_page(self, page, bits):
        """
        Read a page in SRAM Cypress CY6226NLL
        :param page:
        :param bits:
        :return:
        """
        self.send_command_read_page(page)
        res = self.check_serial_data()
        return self.process_command(res, bits)

    def read_byte(self, byt):
        """
        Read a byte in SRAM Cypress CY6226NLL
        :param byt:
        :return:
        """
        ret = ReadByteResult
        while ret.location != byt:
            self.send_command_read_byte(byt)
            res = self.check_serial_data()
            ret = self.process_command(res)
            if isinstance(ret, ReadByteResult):
                if ret.location == byt:
                    return ret

    def read_bit(self, bit):
        """
        Read a bit in SRAM Cypress CY6226NLL
        :param bit: bit address
        :return:
        """
        ret = ReadBitResult
        while ret.location != bit:
            self.send_command_read_bit(bit)
            res = self.check_serial_data()
            ret = self.process_command(res)
            if isinstance(ret, ReadBitResult):
                if ret.location == bit:
                    return ret

    def get_bits_with_value(self, bits, delay=2):
        """
        Retrieve bit values given bit addresses in SRAM Cypress CY6226NLL
        :param bits: bit addresses
        :param delay:
        :return: dict contains pair of bit address and bit value
        """
        bit_values = {}
        self.turn_off_sram()
        time.sleep(delay)

        # initialize
        self.turn_on_sram()
        for i in bits:
            r = self.read_bit(int(i))
            bit_values[i] = r
        self.turn_off_sram()
        time.sleep(delay)

        return bit_values

    def try_data_remanence(self, delay, write_ones):
        """
        Do bit selection algorithm using data remanence approach
        :param delay:
        :param write_ones: set true if want to write one to all address and get locations of strong zeroes
        :return: result[0] = delay
        result[0] = delay
        result[1] = zeroes count
        result[2] = ones count
        result[3] = strong bits locations
        """
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

    def write_page(self, page, is_one, bits):
        """
        Write a page in SRAM Cypress CY62256NLL
        :param page:
        :param is_one: set true if want to write one to all locations in that page
        :param bits:
        :return:
        """
        self.send_command_write_page(page, is_one)
        res = self.check_serial_data()
        return self.process_command(res, bits)

    def write_byte(self, byt, data=0xFF):
        """
        Write a byte in SRAM Cypress CY62256NLL
        :param byt:
        :param data:
        :return:
        """
        self.send_command_write_byte(byt, data=data)
        res = self.check_serial_data()
        return self.process_command(res)

    def write_pages(self, is_ones):
        """
        Write all pages / all locations in SRAM Cypress CY62256NLL
        :param is_ones:
        """
        bits = []
        current_page = 0
        total_page = 1024

        while current_page < total_page:
            self.write_page(current_page, is_ones, bits)
            current_page = current_page + 1

    def new_challenges_on_sd(self):
        """
        Set up to add new challenge in microSD through Arduino
        """
        self.send_command_new_challenges()
        res = self.check_serial_data()
        return self.process_command(res)

    def append_challenges_on_sd(self, address):
        """
        Append address to a challenge in microSD through Arduino
        :param address:
        """
        self.send_command_append_challenges(address)
        res = self.check_serial_data()
        self.process_command(res)

    def close_challenges_on_sd(self):
        """
        Ask Arduino to close challenge file which located in microSD
        """
        self.send_command_finish_writing_challenges()
        res = self.check_serial_data()
        self.process_command(res)

    def write_challenges_to_sd(self, challenges):
        """
        Write a challenge to microSD through Arduino
        :param challenges:
        """
        a = self.new_challenges_on_sd()
        if a == 0:
            print("Error opening challenge file on microSD")
            exit(1)
        for i in challenges:
            self.append_challenges_on_sd(int(i))
        self.close_challenges_on_sd()

    def generate_helper_data_on_sd(self):
        """
        Ask arduino to generate helper data
        """
        self.send_command_generate_helper_data()
        res = self.check_serial_data()
        self.process_command(res)

    def get_keys(self):
        """
        Ask Arduino to retrieve the PUF generated key
        :return:
        """
        self.send_command_get_keys()
        res = self.check_serial_data()
        return self.process_command(res)

    def check_if_it_is_working(self, delay=5):
        """
        Check whether the SRAM Cypress CY62256NLL is working properly or not by writing values then reading the values
        again. SRAM is working fine if all values read are the same as the values written
        :param delay:
        """
        self.turn_on_sram(is_delay=True)
        time.sleep(0.5)

        data = 0xFF
        count_wrong = 0
        for i in range(0, 1024):

            bits = []
            a = self.write_page(int(i), True, bits)
            b = self.read_page(int(i), bits)
            for j in b.bytes:
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
