from PUF.NeighborAnalysis import StableBitsCalculator, WindowBitsCalculator
from PUF import Tools

# -------------------------------------------------------------------------------------------------------
# ---------------------------------------- MICROCHIP 23LC1024--------------------------------------------
# -------------------------------------------------------------------------------------------------------
# ---------------------------------------- CALCULATE THE STABLE BITS ------------------------------------
folders = ['23LC1024/all-locations/A/2018128', '23LC1024/all-locations/A/2018129']
h = StableBitsCalculator.StableBitsCalculator()
files = h.convert_folders(folders, length=1048576)
bits = h.locate_stable_bit(files[:500], length=1048576)
# Tools.save_to_file(bits, "../CY62256NLL/bits-" + Tools.create_today_date())

# ---------------------------------------- CALCULATE THE RANK OF BITS -----------------------------------
# bits = Tools.read_bits_from_file("23LC1024/bits-2018130")
calculator = WindowBitsCalculator.WindowBitsCalculator()
calculator.initialize_bits_score(bits=bits, length=1048576)
calculator.calculate(max_length=1048576, window=5)
score = calculator.calculate_count_each()
for i in score:
    print(str(i) + ", " + str(score[i]))

# ---------------------------------------- GET 4662 STABLE BITS TO TEST FURTHER -----------------------------------
filtered_count = calculator.get_filtered_index_by_count(63*37*2)
# Tools.save_to_file(filtered_count, "../CY62256NLL/bitsfiltered-" + Tools.create_today_date())


# -------------------------------------------------------------------------------------------------------
# ---------------------------------------- CYPRESS CY62256NLL--------------------------------------------
# -------------------------------------------------------------------------------------------------------
# # ---------------------------------------- CALCULATE THE STABLE BITS ------------------------------------
# folders = ['CY62256NLL/all-locations/A/2018-03-31', 'CY62256NLL/all-locations/A/2018-02-06',
#            'CY62256NLL/all-locations/A/2018-02-07', ]
# h = StableBitsCalculator.StableBitsCalculator()
# files = h.convert_folders(folders, length=262144)
# bits = h.locate_stable_bit(files[:500], length=262144)
# # # Tools.save_to_file(bits, "../CY62256NLL/bits-" + Tools.create_today_date())
#
# # ---------------------------------------- CALCULATE THE RANK OF BITS -----------------------------------
# # bits = Tools.read_bits_from_file("CY62256NLL/neighbor-all-location/A/bits-2018-04-01-2ß")
# calculator = WindowBitsCalculator.WindowBitsCalculator()
# calculator.initialize_bits_score(bits=bits)
# calculator.calculate(max_length=262144, window=15)
# score = calculator.calculate_count_each()
# for i in score:
#     print(str(i) + ", " + str(score[i]))
#
# # ---------------------------------------- GET 4662 STABLE BITS TO TEST FURTHER -----------------------------------
# filtered_count = calculator.get_filtered_index_by_count(63*37*2)
# # Tools.save_to_file(filtered_count, "../CY62256NLL/bitsfiltered-" + Tools.create_today_date())
