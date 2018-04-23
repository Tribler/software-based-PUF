from __future__ import division

import distance
import os
import itertools
from os.path import isfile, join

class StableBitsCalculator:
    scale = 16
    num_of_bits = 8

    def convert_file_hex_to_bin(self, name):
        f = open(name).readlines()
        f = [x.strip() for x in f]
        convert = []
        for a in f:
            if a:
                convert.append(bin(int(a, self.scale))[2:].zfill(self.num_of_bits))
        result = ""
        for a in convert:
            for b in a:
                result += b
        return result

    def read_file(self, name):
        f = open(name).readlines()
        f = [x.strip() for x in f]
        result = ""
        for a in f:
            for b in a:
                result += b
        return result

    def gen_converted_files(self, folders, length=1048576, convert=False):
        files = {}
        for folder in folders:
            for filename in os.listdir(folder + "/"):
                if convert:
                    converted = self.convert_file_hex_to_bin(folder + "/" + filename)
                else:
                    converted = self.read_file(folder + "/" + filename)
                len_converted = len(converted)
                if len_converted == length:
                    files[folder + "/" + filename] = converted
                    # files.append(converted)
        return files

    def convert_folder(self, folder, length=1048576, convert=False):
        files = []
        for filename in os.listdir(folder):
            a = folder + "/" + filename
            if not isfile(a):
                fs = self.convert_folder(a, length, convert)
                files.extend(fs)
            else:
                b = self.convert_file(a, length, convert)
                if b is not None:
                    files.append(b)
        return files

    def convert_folders(self, folders, length=1048576, convert=False):
        files = []
        for folder in folders:
            files.extend(self.convert_folder(folder, length, convert))
        return files

    def convert_file(self, file, length=1048576, convert=False):
        if convert:
            converted = self.convert_file_hex_to_bin(file)
        else:
            converted = self.read_file(file)
        len_converted = len(converted)
        if len_converted == length:
            return converted
        else:
            return None

    def calculate_hamming_distance(self, files, length=1048576):
        total = 0
        count = 0
        highest = 0
        highest_pct = 0
        lowest = length
        lowest_pct = 100
        keys = list(files.keys())
        a = files[keys[0]]
        high = 0
        name = ""
        print(keys[0])
        for c, d in itertools.combinations(files, 2):
        # for c in keys[1:]:
        #     b = files[c]
            a = files[c]
            b = files[d]
            if len(a) != length and len(b) != length:
                continue
            dis = distance.hamming(a, b)
            pct = (dis / length) * 100
            total += pct
            count += 1
            if highest < dis:
                highest = dis
                highest_pct = pct
                high = count
                # name = c
            if lowest > dis:
                lowest = dis
                lowest_pct = pct
            print(str(c) + "-" + str(d) + " : " + str(dis) + ", " + str(pct) + "%")
        return [total/count, highest, lowest, highest_pct,lowest_pct, high, name]

    def calculate_hamming_distance_list(self, files, length=1048576):
        total = 0
        count = 0
        highest = 0
        highest_pct = 0
        lowest = length
        lowest_pct = 100
        a = files[0]
        high = 0
        name = ""
        for a, b in itertools.combinations(files, 2):
        # for c in keys[1:]:
        #     b = files[c]
            if len(a) != length and len(b) != length:
                continue
            dis = distance.hamming(a, b)
            pct = (dis / length) * 100
            total += pct
            count += 1
            if highest < dis:
                highest = dis
                highest_pct = pct
                high = count
            if lowest > dis:
                lowest = dis
                lowest_pct = pct
            print(str(dis) + ", " + str(pct) + "%")
        return [total/count, highest, lowest, highest_pct,lowest_pct, high]

    def locate_stable_bit(self, files, length=1048576):
        start = 0
        bit_pufs = []
        for i in range(start, start + length):
            bit_pufs.append(i)

        bit_to_remove = []
        for a in files[1:]:
            for i in bit_pufs:
                if (a[i] != files[0][i]):
                    bit_to_remove.append(i)
                    continue
            for i in bit_to_remove:
                bit_pufs.remove(i)
            bit_to_remove = []

        return bit_pufs

    def print_summary_bit(self, bit_pufs, original_length=1048576):
        print(len(bit_pufs))
        print(len(bit_pufs) / original_length)
        print(original_length - len(bit_pufs))
        print(original_length)