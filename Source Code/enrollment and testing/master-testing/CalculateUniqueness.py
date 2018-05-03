import os
import itertools
from PUF import Tools, Analyzer, DataRemanenceCalculator
import distance
h = Analyzer.Analyzer()


def read_folder(folder):
    result = []
    for f in os.listdir(folder + "/"):
        file = folder + "/" + f
        if os.path.isdir(file):
            result.extend(read_folder(file))
        else:
            if not file.endswith(".DS_Store"):
                result.append(read_file(file))
    return result


def read_file(file):
    return Tools.read_bits_from_file_and_merge(file)


def calculate_uniquessness(min_index_SRAM, max_index_SRAM):
    files_per_folder = {}

    for k in [chr(i) for i in range(ord(min_index_SRAM), ord(max_index_SRAM) + 1)]:
        folder = 'Values/' + k
        files = read_folder(folder)
        files_per_folder[k] = files

    inter_hd = []
    for a, b in itertools.combinations(files_per_folder, 2):
        files1 = files_per_folder[a]
        files2 = files_per_folder[b]
        dis = h.calculate_inter_hamming_distance(files1, files2, length=4662)
        inter_hd.append(dis)

    average = sum(inter_hd) / len(inter_hd)
    max_distance = max(inter_hd)
    min_distance = min(inter_hd)

    return [average, max_distance, min_distance]


result = calculate_uniquessness(min_index_SRAM='A', max_index_SRAM='B')

print("Average inter fractional hamming distance: " + str(result[0]))
print("Maximum inter fractional hamming distance: " + str(result[1]))
print("Minimum inter fractional hamming distance: " + str(result[2]))
