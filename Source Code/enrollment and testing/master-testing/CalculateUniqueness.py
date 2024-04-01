import os
import itertools
from PUF import Tools, Analyzer, DataRemanenceCalculator
import distance     # from scipy.spatial import distance

h = Analyzer.Analyzer()

def read_folder(folder):
    result = []
    for f in os.listdir(folder + os.sep):       # use os.sep instead of hardcoding '/'
        file = folder + os.sep + f
        if os.path.isdir(file):
            result.extend(read_folder(file))
        else:
            if not file.endswith(".DS_Store"):
                result.append(read_file(file))
    return result


def read_file(file):
    return Tools.read_bits_from_file_and_merge(file)


def get_index_list() -> list:
    dir_name = 'Values' + os.sep
    result = [i for i in os.listdir(dir_name) if os.path.isdir(os.path.join(dir_name, i))]      # directories only
    return result


def calculate_uniqueness():
    files_per_folder = {}
    dir_name = 'Values' + os.sep        # 'Values/'

    index_list = get_index_list()
    if len(index_list) > 0:
        min_index_sram = min(index_list)
        max_index_sram = max(index_list)
    else:
        print('no index directories found... exit')
        return None

    for k in [chr(i) for i in range(ord(min_index_sram), ord(max_index_sram) + 1)]:
        folder = dir_name + k
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


result = calculate_uniqueness()
if result is not None:
    print("Average inter fractional hamming distance: " + str(result[0]))
    print("Maximum inter fractional hamming distance: " + str(result[1]))
    print("Minimum inter fractional hamming distance: " + str(result[2]))
