import os, errno, collections
import sys
import datetime

def save_to_tmp(data, tmpfile, with_comma=False):
    """
    Uses partial code written by Ade Setyawan Sajim but saves
    data to a tempfile rather than a persistent file.
    :param data: data to write to tmpfile
    :param tmpfile: named tempfile, created with mode='w+t' (write+textfile)
    :param with_comma: formatting flag
    :return: 0 on success, 1 on error
    """
    if not tmpfile.closed and tmpfile.mode == 'w+t':        # tmpfile is open and mode is 'w+t'
        if isinstance(data, collections.Iterable):
            if not with_comma:
                for i in data:
                    print(i, file=tmpfile)
            else:
                j = 0
                for i in data:
                    print(i, file=tmpfile, end=", ")
                    j += 1
                    if j%16 == 0:
                        print(file=tmpfile)
        else:
            print(data, file=tmpfile)
        tmpfile.flush()
        return 0
    else:       # tempfile should be open with w+t attributes
        print("tempfile error\n")
        return 1


def save_to_file(data, filename, with_comma=False):
    dir_name = os.path.dirname(filename)
    if not os.path.exists(dir_name) and len(dir_name) > 0:
        try:
            os.makedirs(dir_name)
        except OSError as exc:  # Guard against race condition
            if exc.errno != errno.EEXIST:
                raise
    with open(filename, "w") as text_file:
        if isinstance(data, collections.Iterable):
            if not with_comma:
                for i in data:
                    print(i, file=text_file)
            else:
                j = 0
                for i in data:
                    print(i, file=text_file, end=", ")
                    j += 1
                    if j%16 == 0:
                        print(file=text_file)
        else:
            print(data, file=text_file)


def read_bits_from_file(filename, is_separated_by_comma=False):
    with open(filename) as f:
        content = f.readlines()
        if not is_separated_by_comma:
            content = [x.strip() for x in content]
            return content
        else:
            temp = []
            content = [x.replace(",", "").strip() for x in content]
            j = 0
            for x in content:
                for y in x.split():
                    temp.append(y)
                    j += 1
            #         print(y, end=", ")
            #         if j%8 == 0:
            #             print()
            # print(j)
            return temp


def read_bits_from_file_and_merge(name):
    with open(name) as e:
        f = e.readlines()
        f = [x.strip() for x in f]
        result = ""
        for a in f:
            for b in a:
                result += b
        return result


def read_bits_value_from_file(filename):
    with open(filename) as f:
        content = f.readlines()
        temp = {}
        j = 0
        for x in content:
            x = x.strip()
            y = x.split(", ")
            # temp.append([int(y[0]), int(y[1])])
            temp[int(y[0])] = int(y[1])
        return temp


def create_today_date():
    now = datetime.date.today()
    return str(now)


def read_remanence_from_file(filename):
    with open(filename) as f:
        content = f.readlines()
        temp = []
        content = [x.strip() for x in content]
        j = 0
        for x in content:
            a = x.split(" - ")
            b = a[1].split(", ")
            temp.append([a[0], b[0], b[1]])
                # j += 1
                # print(y)
                # if j%8 == 0:
                #     print()
        # print(j)
        return temp


def read_score(filename, max=-1):
    res = {}
    with open(filename) as f:
        content = f.readlines()
        content = [x.strip() for x in content]
        for i in content:
            a = i.split(",")
            if (max != -1):
                res[int(a[0])] = int(a[1]) / max * 100
            else:
                res[int(a[0])] = int(a[1])
    return res


def read_dict(filename):
    res = {}
    with open(filename) as f:
        content = f.readlines()
        content = [x.strip() for x in content]
        for i in content:
            a = i.split(",")
            res[a[0]] = float(a[1])
    return res


def print_list(lst, end="\n"):
    for i in lst:
        print(str(i), end=end)


def print_dict(dct, end="\n"):
    for i in dct:
        print(str(i) + ", " + str(dct[i]), end=end)


def convert_dict_to_list(dct, separator=", "):
    dct_string = []
    for i in dct:
        dct_string.append(i + separator + str(dct[i]))
    return dct_string

def sys_path_append(paths):
    """
    Wrapper for sys.path.append that includes checks.  Accepts single path
    string or a list of strings to temporarily append.  This is a duplicate
    of sys_path_append found in the wrapper module.
    :param paths: string or list of strings containing paths to add
    :return: 0 if all succeed, 1 on error
    """
    if isinstance(paths, str):
        paths = [paths]     # convert string to list for single path
    for p in paths:
        present = False     # flag indicating status of path in sys.path
        if not os.path.exists(p):    # check for non-existent path
            return 1
        path = os.path.abspath(p)
        for i in sys.path:
            x = os.path.abspath(i)
            if path in (x, x+os.sep):     # already in sys.path
                present = True
                break
        if not present:
            sys.path.append(path)
            print(path, "added to sys.path (temporary)")
    return 0
