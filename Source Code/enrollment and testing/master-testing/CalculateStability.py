import os
import itertools
from PUF import Tools, Analyzer, DataRemanenceCalculator

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


def calculate_stability(index_SRAM):
    folder = 'Values/' + index_SRAM
    files = read_folder(folder)
    return h.calculate_intra_hamming_distance_between_elements(files, length=4662)


result = calculate_stability(index_SRAM='C')
print("Average Pct : " + str(result[0]))
print("Highest Pct : " + str(result[3]))
print("Lowest Pct \t: " + str(result[4]))
