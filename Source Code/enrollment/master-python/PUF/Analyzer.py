from __future__ import division

import distance
import os
import itertools
import numpy
import matplotlib.pyplot as plt


class Analyzer:
    def calculate_intra_hamming_distance(self, files, length=1048576):
        total = 0
        count = 0
        highest = 0
        highest_pct = 0
        lowest = length
        lowest_pct = 100
        keys = list(files.keys())
        a = files[keys[0]]
        distances = {}
        alphabet = 'B'
        for c in keys[1:]:
            b = files[c]
            if len(a) != length and len(b) != length:
                continue
            dis = distance.hamming(a, b)
            pct = (dis / length) * 100
            total += pct
            count += 1
            name = "A-" + alphabet
            distances[name] = pct
            if highest < dis:
                highest = dis
                highest_pct = pct
            if lowest > dis:
                lowest = dis
                lowest_pct = pct
            alphabet = chr(ord(alphabet) + 1)
            # print(str(dis) + ", " + str(pct) + "%")
        return [total / count, highest, lowest, highest_pct, lowest_pct, distances]

    def calculate_intra_hamming_distance_between_elements(self, files, length=1048576):
        total = 0
        count = 0
        highest = 0
        highest_pct = 0
        lowest = length
        lowest_pct = 100
        distances = {}
        if isinstance(files, dict):
            for a1, b1 in itertools.combinations(files, 2):
                a = files[a1]
                b = files[b1]
                if len(a) != length and len(b) != length:
                    continue
                dis = distance.hamming(a, b)
                pct = (dis / length) * 100
                total += pct
                count += 1
                name = a1 + "-" + b1
                distances[name] = pct
                if highest < dis:
                    highest = dis
                    highest_pct = pct
                if lowest > dis:
                    lowest = dis
                    lowest_pct = pct
                    # print(str(dis) + ", " + str(pct) + "%")
        else:
            for a, b in itertools.combinations(files, 2):
                if len(a) != length and len(b) != length:
                    continue
                dis = distance.hamming(a, b)
                pct = (dis / length) * 100
                total += pct
                count += 1
                if highest < dis:
                    highest = dis
                    highest_pct = pct
                if lowest > dis:
                    lowest = dis
                    lowest_pct = pct
                    # print(str(dis) + ", " + str(pct) + "%")
        return [total / count, highest, lowest, highest_pct, lowest_pct, distances]

    def calculate_inter_hamming_distance(self, files1, files2, length=1048576):
        total = 0
        count = 0
        if isinstance(files1, list) and isinstance(files2, list):
            for i in files1:
                for j in files2:
                    dis = distance.hamming(i, j)
                    pct = (dis / length) * 100
                    total += pct
                    count += 1
        else:
            dis = distance.hamming(files1, files2)
            pct = (dis / length) * 100
            total += pct
            count += 1

        return total / count

    def gen_graph_hamming_distance(self, avg_distance, ymin=0, ymax=10, xlbl='Voltage',
                                   ylbl='Fractional Hamming Distance (%)'):
        xs = []
        ys = []
        for i in avg_distance:
            # xs.append(i[-3:])
            xs.append(i)
            ys.append(avg_distance[i])

        fig, ax = plt.subplots()
        # fig.set_size_inches(18.5, 10.5)

        ax.plot(xs, ys)

        ax.set(xlabel=xlbl, ylabel=ylbl)
        ax.set_ylim([ymin, ymax])
        ax.grid()

        for i, j in zip(xs, ys):
            ax.annotate(str(round(j, 2)), xy=(i, j + 0.1))

        plt.show()

    def gen_graph_rank(self, values, ymin=0, ymax=10, xlbl='Voltage', ylbl='Hamming Distance %', limit=10):
        xs = []
        ys = []
        a = 0
        for i in values:
            xs.append(i)
            ys.append(values[i])
            a += 1
            if a > limit:
                break

        xs = ['≥16', '15', '14', '13', '12', '11', '10', '9', '8']
        width = .35
        ind = numpy.arange(len(ys))

        fig, ax = plt.subplots()
        # ax.bar(xs, ys)
        plt.bar(ind, ys, width=width)
        plt.xticks(ind + width / 2, xs)

        fig.autofmt_xdate()
        # ax.plot(xs, ys)

        # ax.set(xlabel=xlbl, ylabel=ylbl)
        # ax.set_xticks()
        ax.set_ylim([ymin, ymax])
        # ax.grid()

        # for i, v in enumerate(ys):
        #     ax.text(v + 3, i + .25, str(v), color='blue', fontweight='bold')

        a = 0
        for i in ax.patches:
            # get_x pulls left or right; get_height pushes up or down
            ax.text(i.get_x() - .053, i.get_height() + 100.9, str(int(ys[a])), fontsize=9)
            a += 1

        # for i, j in zip(xs, ys):
        #     ax.annotate(str(round(j, 2)), xy=(i, j + 0.2))

        plt.show()

    def gen_multiple_graph_hamming_distance(self, data, ymin=0, ymax=10, xlbl='Voltage', ylbl='Hamming Distance %',
                                            is_vertical=True):
        if is_vertical:
            fig, ax = plt.subplots(nrows=len(data), figsize=(12, 5))
        else:
            fig, ax = plt.subplots(ncols=len(data), figsize=(12, 5))
        # fig.set_size_inches(15, 15)

        a = 0
        for avg_distance in data:
            xs = []
            ys = []

            for i in avg_distance:
                xs.append(i[-3:])
                ys.append(avg_distance[i])

            ax[a].plot(xs, ys)

            for i, j in zip(xs, ys):
                ax[a].annotate(str(round(j, 2)), xy=(i, j + 0.2))

            ax[a].set(xlabel=xlbl, ylabel=ylbl)
            ax[a].set_ylim([ymin, ymax])
            ax[a].grid()
            a += 1

        plt.show()

    def gen_graph(self, data, ymin=0, ymax=10, xmin=-1, xmax=-1, xlbl='', ylbl=''):
        xs = []
        ys = []
        for i in data:
            xs.append(i)
            ys.append(data[i])

        fig, ax = plt.subplots()
        ax.plot(xs, ys)

        ax.set(xlabel=xlbl, ylabel=ylbl)
        ax.set_ylim([ymin, ymax])
        if xmin != -1 and xmax != -1:
            ax.set_xlim([xmin, xmax])
            plt.xticks(xs)
        ax.grid()

        plt.show()

    def calculate_percentage_stable_bits(self, filename):
        res = {}
        res_mean = {}
        with open(filename) as f:
            content = f.readlines()
            for i in content:
                j = i[1:-2]
                a = j.split(", ")
                # print(a)
                if float(a[0]) in res:
                    res[float(a[0])].append(int(a[1]))
                else:
                    res[float(a[0])] = []
                    res[float(a[0])].append(int(a[1]))
            for i in res:
                j = res[i]
                res_mean[i] = (sum(j) / len(j)) / 4662 * 100
        return res_mean
