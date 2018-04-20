import time

from random import shuffle
from PUF import SerialPUF, Tools


def get_strong_bits_by_goal(goal, initial_delay=0.2, step_delay=0.5, write_ones=True):
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
    is_prev_larger = False

    current_goal = 0
    delays = []

    while not (goal + 50 > current_goal > goal - 50):
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
            if is_prev_larger:
                step_delay = step_delay / 2
            delay = round(delay + step_delay, 3)
            if delay in delays:
                step_delay = step_delay / 2
                delay = round(delay + step_delay, 3)

            is_prev_larger = False
        if current_goal > goal + 100:
            if is_prev_larger:
                step_delay = step_delay / 2
            delay = round(delay - step_delay, 3)
            if delay in delays:
                step_delay = step_delay / 2
                delay = round(delay + step_delay, 3)
            is_prev_larger = True

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


# Initialize serial connection to connect to Arduino
serialPUF = SerialPUF.SerialPUF()
if not serialPUF.connect('/dev/cu.usbmodem1411', 115200):
    print("Error connecting to Arduino")
    exit(1)
time.sleep(2)

# -------------------------------------------------------------------------------------------------------
# ---------------------------------------- CHECK IF IT'S WORKING -----------------------------------
# -------------------------------------------------------------------------------------------------------

# serialPUF.check_if_it_is_working()

# -------------------------------------------------------------------------------------------------------
# --------------------------------------- DATA REMANENCE TRIAL ------------------------------------------
# -------------------------------------------------------------------------------------------------------

# strong_ones = get_strong_bits_by_goal(goal=2300, write_ones=False, initial_delay=0.34, step_delay=0.01)
strong_ones = get_strong_bits_by_time(delay=0.31, write_ones=False)
# strong_zeros = get_strong_bits_by_goal(goal=2300, write_ones=True, initial_delay=0.34, step_delay=0.01)
strong_zeros = get_strong_bits_by_time(delay=0.31, write_ones=True)

# --------------------------------------------------------------------------------
# -------------------- GENERATE CHALLENGES ---------------------------------------
# --------------------------------------------------------------------------------

x = []
x.extend(strong_zeros[:2331])
x.extend(strong_ones[:2331])
x = list(map(int, x))

shuffle(x)
serialPUF.write_challenges_to_sd(x[:37*63])
Tools.save_to_file(x[:37*63], "challenge.txt", with_comma=True)

# res = Tools.read_bits_from_file("challenge.txt", is_separated_by_comma=True)
# x = []
# for i in res:
#     x.append(int(i))
# serialPUF.write_challenges_to_sd(x)


# --------------------------------------------------------------------------------
# -------------------- GENERATE HELPER DATA --------------------------------------
# --------------------------------------------------------------------------------
serialPUF.generate_helper_data_on_sd()