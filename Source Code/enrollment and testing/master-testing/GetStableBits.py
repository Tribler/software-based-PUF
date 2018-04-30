import time

from random import shuffle
from PUF import SerialPUF, Tools


def get_strong_bits_by_goal(goal, initial_delay=0.3, step_delay=0.005, write_ones=True):
    """
    Bit selection algorithm using data remanence approach
    :param goal: number of strong bits desired
    :param initial_delay: initial delay
    :param step_delay: initial step delay
    :param write_ones: write one to all location. Set to true if strong zeroes are the goal.
            Otherwise set to false
    :return: location of strong bits
    """
    delay = initial_delay
    strong_bits = []

    current_goal = 0
    delays = []

    while current_goal < goal:
        print("DATA REMANENCE - delay: " + str(delay))
        delays.append(delay)

        result = serialPUF.try_data_remanence(delay, write_ones)
        strong_bits = result[3]
        if not write_ones:
            current_goal = result[2]
        else:
            current_goal = result[1]
        print(current_goal)

        if current_goal < goal:
            delay = delay + step_delay

        print()

    return strong_bits


def get_strong_bits_by_time(delay, write_ones=True):
    """
    Bit selection algorithm using data remanence approach
    :param goal: number of strong bits desired
    :param initial_delay: initial delay
    :param step_delay: initial step delay
    :param write_ones: write one to all location. Set to true if strong zeroes are the goal.
            Otherwise set to false
    :return: location of strong bits
    """
    delays = []

    print("DATA REMANENCE - delay: " + str(delay))
    delays.append(delay)

    result = serialPUF.try_data_remanence(delay, write_ones)
    strong_bits = result[3]
    if not write_ones:
        current_goal = result[2]
    else:
        current_goal = result[1]
    print(current_goal)

    return strong_bits


is_sram_23lc1024 = False

# Initialize serial connection to connect to Arduino
serialPUF = SerialPUF.SerialPUF()
if not serialPUF.connect('/dev/cu.usbmodem1411', 115200):
    print("Error connecting to Arduino")
    exit(1)
time.sleep(2)

# serialPUF.check_if_it_is_working()

strong_ones = get_strong_bits_by_goal(goal=2331, initial_delay=0.335, write_ones=False)
# strong_ones = get_strong_bits_by_time(delay=0.31, write_ones=False)
strong_zeros = get_strong_bits_by_goal(goal=2331, initial_delay=0.335, write_ones=True)
# strong_zeros = get_strong_bits_by_time(delay=0.31, write_ones=True)

x = []
x.extend(strong_zeros[:2331])
x.extend(strong_ones[:2331])
x = list(map(int, x))

Tools.save_to_file(x, "strongbits.txt", with_comma=True)
