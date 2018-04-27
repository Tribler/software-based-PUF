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


# -------------------------------------------------------------------------------------------------------
# ----------------------------------- CALCULATE THE INTRA HAMMING DISTANCE ------------------------------
# -------------------------------------------------------------------------------------------------------

# # # ----------------------------------- 23LC1024 NORMAL ------------------------------------------------------------
# avg_distance_all = {}
#
# for k in [chr(i) for i in range(ord('A'), ord('J') + 1)]:
#     folder = '/Users/ades17/Documents/Thesis/Software/puf_patent_free_arduino/data final/23LC1024/all-locations/' + k\
#              + '/NORMAL'
#     files = read_folder(folder)
#     a = h.calculate_intra_hamming_distance_between_elements(files)
#     avg_distance = a[0]
#     avg_distance_all[k] = avg_distance
#
# sum = 0
# for i in avg_distance_all:
#     sum += avg_distance_all[i]
#
# print(sum / len(avg_distance_all))
#
# h.gen_graph_hamming_distance(avg_distance_all, ymin=0, ymax=20, xlbl='Chip')
#
# # ----------------------------------- 23LC1024 VOLTAGE VARIATION --------------------------------------------------
# avg_distance_all = {}
#
# for k in [chr(i) for i in range(ord('A'), ord('J') + 1)]:
#     print(k)
#     folder = '/Users/ades17/Documents/Thesis/Software/puf_patent_free_arduino/data final/23LC1024/all-locations/' + k \
#              + '/VOLTAGE'
#     files = read_folder(folder)
#     a = h.calculate_intra_hamming_distance_between_elements(files)
#     avg_distance_all[k] = a[0]
#
# sum = 0
# for i in avg_distance_all:
#     # print(i + ", " + str(avg_distance_all[i]))
#     sum += avg_distance_all[i]
#
# print(sum / len(avg_distance_all))
#
# h.gen_graph_hamming_distance(avg_distance_all, ymin=0, ymax=20, xlbl='Chip')
#
# # ----------------------------------- CY62256NLL NORMAL ------------------------------------------------------------
# avg_distance_all = {}
#
# for k in [chr(i) for i in range(ord('A'), ord('E') + 1)]:
#     folder = '/Users/ades17/Documents/Thesis/Software/puf_patent_free_arduino/data final/CY62256NLL/all-locations/' + k\
#              + '/NORMAL'
#     files = read_folder(folder)
#     a = h.calculate_intra_hamming_distance_between_elements(files, length=262144)
#     avg_distance = a[0]
#     avg_distance_all[k] = avg_distance
#
# sum = 0
# for i in avg_distance_all:
#     sum += avg_distance_all[i]
#
# print(sum / len(avg_distance_all))
#
# h.gen_graph_hamming_distance(avg_distance_all, ymin=0, ymax=20, xlbl='Chip')
#
# # # ----------------------------------- CY62256NLL VOLTAGE VARIATION --------------------------------------------------
# avg_distance_all = {}
#
# for k in [chr(i) for i in range(ord('A'), ord('E') + 1)]:
#     print(k)
#     folder = '/Users/ades17/Documents/Thesis/Software/puf_patent_free_arduino/data final/CY62256NLL/all-locations/' + k \
#              + '/VOLTAGE'
#     files = read_folder(folder)
#     a = h.calculate_intra_hamming_distance_between_elements(files, length=262144)
#     avg_distance_all[k] = a[0]
#
# sum = 0
# for i in avg_distance_all:
#     # print(i + ", " + str(avg_distance_all[i]))
#     sum += avg_distance_all[i]
#
# print(sum / len(avg_distance_all))
#
# h.gen_graph_hamming_distance(avg_distance_all, ymin=0, ymax=20, xlbl='Chip')
#
# # ----------------------------------- 23LC1024 STABLE RANK ----------------------------------------
# folder = '../23LC1024/stable-neighbor/'
# files = {}
# for filename in os.listdir(folder + "/"):
#     files[folder + "/" + filename] = Tools.read_bits_from_file_and_merge(folder + "/" + filename)
#
# a = h.calculate_intra_hamming_distance(files, length=4662)
# Tools.print_dict(a[5])
# distances_string = Tools.convert_dict_to_list(a[5])
# Tools.save_to_file(distances_string, "../23LC1024/distances-stable-neighbor-" + Tools.create_today_date())
#
# # ----------------------------------- 23LC1024 STABLE REMANENCES -----------------------------------
# folder = '../23LC1024/stable-remanences/'
# files = {}
# for filename in os.listdir(folder + "/"):
#     files[folder + "/" + filename] = Tools.read_bits_from_file_and_merge(folder + "/" + filename)
#
# a = h.calculate_intra_hamming_distance(files, length=4662)
# # Tools.print_dict(a[5])
# distances_string = Tools.convert_dict_to_list(a[5])
# Tools.save_to_file(distances_string, "../23LC1024/distances-stable-remanences-" + Tools.create_today_date())
# b = Tools.read_dict("../23LC1024/distances-stable-remanences-" + Tools.create_today_date())
#
# h.gen_graph_hamming_distance(b, xlbl='Distances Between', ymax=70)
#
# # ----------------------------------- CY62256NLL STABLE RANK ----------------------------------------
# folder = '../CY62256NLL/stable-neighbor/'
# files = {}
# for filename in os.listdir(folder + "/"):
#     files[folder + "/" + filename] = Tools.read_bits_from_file_and_merge(folder + "/" + filename)
#
# a = h.calculate_intra_hamming_distance(files, length=4662)
# # Tools.print_dict(a[5])
# distances_string = Tools.convert_dict_to_list(a[5])
# Tools.save_to_file(distances_string, "../CY62256NLL/distances-stable-neighbor-" + Tools.create_today_date())
# b = Tools.read_dict("../CY62256NLL/distances-stable-neighbor-" + Tools.create_today_date())
#
# h.gen_graph_hamming_distance(b, xlbl='Distances Between', ymax=5)
#
# # ----------------------------------- 23LC1024 STABLE REMANENCES -----------------------------------
# folder = '../CY62256NLL/stable-remanences/'
# files = {}
# for filename in os.listdir(folder + "/"):
#     files[folder + "/" + filename] = Tools.read_bits_from_file_and_merge(folder + "/" + filename)
#
# a = h.calculate_intra_hamming_distance(files, length=4662)
# # Tools.print_dict(a[5])
# distances_string = Tools.convert_dict_to_list(a[5])
# Tools.save_to_file(distances_string, "../CY62256NLL/distances-stable-remanences-" + Tools.create_today_date())
# b = Tools.read_dict("../CY62256NLL/distances-stable-remanences-" + Tools.create_today_date())
#
# h.gen_graph_hamming_distance(b, xlbl='Distances Between', ymax=5)

# -------------------------------------------------------------------------------------------------------
# ----------------------------------- CALCULATE THE INTER HAMMING DISTANCE ------------------------------
# -------------------------------------------------------------------------------------------------------
#
# ----------------------------------- 23LC1024 NORMAL ------------------------------------------------------------
# avg_distance = {}
# files_per_folder = {}
#
# for k in [chr(i) for i in range(ord('A'),ord('J')+1)]:
#     # print(k)
#     folder = '../23LC1024/'+ k +'/2018-02-20/NORMAL'
#     files =  []
#     for filename in os.listdir(folder + "/"):
#         files.append(Tools.read_bits_from_file_and_merge(folder + "/" + filename))
#     files_per_folder[k] = files
#
# inter_hd = []
# for a, b in itertools.combinations(files_per_folder, 2):
#     files1 = files_per_folder[a]
#     files2 = files_per_folder[b]
#     inter_hd.append(h.calculate_inter_hamming_distance(files1,  files2))
#
# print(sum(inter_hd)/len(inter_hd))
#
# # ----------------------------------- 23LC1024 VOLTAGE VARIATION ----------------------------------
# avg_distance = {}
# files_per_folder = {}
#
# for k in [chr(i) for i in range(ord('A'),ord('B')+1)]:
#     # print(k)
#     folder = '../23LC1024/'+ k +'/2018-02-20/'
#     files = []
#     for x in os.listdir(folder):
#         if "NORMAL" in x:
#             continue
#         f = folder + x + "/"
#         for filename in os.listdir(f):
#             files.append(Tools.read_bits_from_file_and_merge(f + "/" + filename))
#     files_per_folder[k] = files
#
# inter_hd = []
# for a, b in itertools.combinations(files_per_folder, 2):
#     files1 = files_per_folder[a]
#     files2 = files_per_folder[b]
#     inter_hd.append(h.calculate_inter_hamming_distance(files1,  files2))
#
# print(sum(inter_hd)/len(inter_hd))

# # ----------------------------------- 23LC1024 RANK BITS ------------------------------------------------------------
# avg_distance = {}
# files = {}
# for k in [chr(i) for i in range(ord('A'),ord('J')+1)]:
#     # print(k)
#     fl = '../23LC1024/bitsfiltered-2018-02-19/'+ k +'/2018-02-20'
#     files[fl] = Tools.read_bits_from_file_and_merge(fl)
#
# # for i in files:
# #     print(i)
# #     print(files[i])
# #     print(len(files[i]))
#
# a = h.calculate_intra_hamming_distance(files, length=4662)
# avg_distance[k] = a[0]
#
# sum = 0
# for i in avg_distance:
#     print(i + ", " + str(avg_distance[i]))
#     sum += avg_distance[i]
# print(sum/len(avg_distance))

# h.gen_graph_hamming_distance(avg_distance, xlbl='Chip')

# ----------------------------------- CY62256NLL NORMAL ------------------------------------------------------------
# avg_distance = {}
# files_per_folder = {}
#
# for k in [chr(i) for i in range(ord('A'),ord('B')+1)]:
#     print(k)
#     folder = '../CY62256NLL/'+ k +'/2018-02-14/NORMAL'
#     files =  []
#     for filename in os.listdir(folder + "/"):
#         files.append(Tools.read_bits_from_file_and_merge(folder + "/" + filename))
#     files_per_folder[k] = files
#
# inter_hd = []
# for a, b in itertools.combinations(files_per_folder, 2):
#     files1 = files_per_folder[a]
#     files2 = files_per_folder[b]
#     inter_hd.append(h.calculate_inter_hamming_distance(files1,  files2, length=262144))
#
# print(sum(inter_hd)/len(inter_hd))

# ----------------------------------- CY62256NLL VOLTAGE VARIATION ------------------------------------------
# avg_distance = {}
# files_per_folder = {}
#
# for k in [chr(i) for i in range(ord('A'),ord('B')+1)]:
#     # print(k)
#     folder = '../CY62256NLL/'+ k +'/2018-02-14/'
#     files = []
#     for x in os.listdir(folder):
#         if "NORMAL" in x:
#             continue
#         f = folder + x + "/"
#         for filename in os.listdir(f):
#             files.append(Tools.read_bits_from_file_and_merge(f + "/" + filename))
#     files_per_folder[k] = files
#
# inter_hd = []
# for a, b in itertools.combinations(files_per_folder, 2):
#     files1 = files_per_folder[a]
#     files2 = files_per_folder[b]
#     inter_hd.append(h.calculate_inter_hamming_distance(files1,  files2, length=262144))
#
# print(sum(inter_hd)/len(inter_hd))

# -------------------------------------------------------------------------------------------------------
# --------------------------------------- RANK GRAPH ---------------------------------------
# -------------------------------------------------------------------------------------------------------

# ************************** 23LC1024 **********************************
b = Tools.read_dict("23LC1024/rank-neighbor-5-2018-04-05")
for i in b:
    print(str(i) + ", " + str(b[i]))
h.gen_graph_rank_23lc1024(b, ymax=700000, limit=6)

# ************************** CY62256NLL **********************************
# b = Tools.read_dict("CY62256NLL/rank-neighbor-15-2018-04-05-edit")
# for i in b:
#     print(str(i) + ", " + str(b[i]))
# h.gen_graph_rank_cy62256nll(b, ymax=6000, limit=8)
#
#
# -------------------------------------------------------------------------------------------------------
# --------------------------------------- DATA REMANENCE GRAPH ---------------------------------------
# -------------------------------------------------------------------------------------------------------

# ************************** 23LC1024 **********************************
# remanence0 = Tools.read_remanence_from_file("23LC1024/remanence0-2018131")
# remanence1 = Tools.read_remanence_from_file("23LC1024/remanence1-2018131")
# a = h.calculate_percentage(remanence0, max=1048576, is_write_one=True)
# b = h.calculate_percentage(remanence1, max=1048576, is_write_one=False)
# h.gen_remanence_graph(a)
# h.gen_remanence_graph(b)

# ************************* CY62256NLL **********************************
# remanence0 = Tools.read_remanence_from_file("CY62256NLL/remanences-count/E/remanence0-2018-04-04")
# remanence1 = Tools.read_remanence_from_file("CY62256NLL/remanences-count/E/remanence1-2018-04-04")
# a = h.calculate_percentage(remanence0, max=262144, is_write_one=False)
# b = h.calculate_percentage(remanence1, max=262144, is_write_one=True)
# h.gen_remanence_graph(a)
# h.gen_remanence_graph(b)
