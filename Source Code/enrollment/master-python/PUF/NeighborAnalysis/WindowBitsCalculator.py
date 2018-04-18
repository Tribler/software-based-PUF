import operator


class WindowBitsCalculator:
    score = {}
    sorted_score = []

    def initialize_bits_score_from_file(self, filename):
        with open(filename) as f:
            content = f.readlines()
        content = [int(x.strip()) for x in content]
        for i in content:
            self.score[i] = 0

    def initialize_bits_score(self, bits, length=262144):
        for i in range(length):
            self.score[int(i)] = 0
        for i in bits:
            self.score[int(i)] += 1
        # for i in bits:
        #     self.score[int(i)] = 1
        print(len(self.score))

    def calculate(self, window=8, max_length=1048576):
        for i in self.score:
            if self.score[i] > 0:
                is_neighbor_stable=True
                r = 1
                while is_neighbor_stable and i+r < max_length and i-r > 0 and r <= window:
                    is_neighbor_stable = self.score[i+r] > 0 and self.score[i-r] > 0
                    if is_neighbor_stable:
                        self.score[i] += 1
                    r += 1

            # if i - window >= 0 and i + window <= max_length:
            #     for j in range(i - 8, i + 8):
            #         if j != i:
            #             if self.score[j] > 0:
            #                 self.score[i] += 1
            #         # if j in self.score and j != i:
            #         #     self.score[i] += 1

        self.sorted_score = sorted(self.score.items(), key=operator.itemgetter(1), reverse=True)

    def calculate_count_each(self):
        score = {}
        for k, v in self.sorted_score:
            if v in score:
                score[v] = score[v] + 1
            else:
                score[v] = 1
        return score

    def print_score(self):
        score = {}
        for k, v in self.sorted_score:
            print(str(k) + ", " + str(v))
            if v in score:
                score[v] = score[v] + 1
            else:
                score[v] = 1

        # print(score)
        # for k, v in score:
        #     print(str(k) + ", " + str(v))


    def get_average(self):
        s = sum(self.score.values())
        return s / len(self.score.values())

    def get_filtered_index_by_min_value(self, min_value):
        filtered = []
        for i in self.score:
            if (self.score[i] > min_value):
                filtered.append(i)
        return filtered

    def get_filtered_index_by_count(self, count):
        i = 0
        filtered = []
        while i < count:
            filtered.append(self.sorted_score[i][0])
            i += 1
        return filtered


