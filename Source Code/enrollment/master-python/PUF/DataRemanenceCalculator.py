import matplotlib.pyplot as plt
import numpy as np

class DataRemanenceCalculator:
    def calculate_percentage(self, data, max=1048576, is_write_one=False):
        res = []
        for i in data:
            if not is_write_one:
                percentage = int(i[1]) / max * 100
            else:
                percentage = int(i[2]) / max * 100
            res.append([float(i[0]), percentage])
        return res

    def gen_graph(self, data, xmin = 0, xmax = 1, ymin = 0, ymax=100):
        xs = []
        ys = []
        for i in data:
            xs.append(i[0])
            ys.append(i[1])

        # Note that using plt.subplots below is equivalent to using
        # fig = plt.figure and then ax = fig.add_subplot(111)
        fig, ax = plt.subplots()
        ax.plot(xs, ys)

        ax.set(xlabel='Time (s)', ylabel='Turn Over Bits % ')
        ax.set_xlim([xmin, xmax])
        ax.set_ylim([ymin, ymax])
        ax.grid()

        plt.show()