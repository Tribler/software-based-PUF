import serial, time, os, errno
import datetime
from struct import *


class ReadPageResult:
    page = -1
    bytes = []

    def __init__(self, res):
        r = unpack('>bbH BBBB BBBB BBBB BBBB BBBB BBBB BBBB BBBB', res)
        self.page = r[2]
        self.bytes = r[3:36]


class ReadByteResult:
    location = -1
    value = -1

    def __init__(self, res, is_sram_23lc1024=True):
        if is_sram_23lc1024:
            r = unpack('>bbLB', res[:7])
            self.location = r[2]
            self.value = r[3]
        else:
            r = unpack('>bbHB', res[:5])
            self.location = r[2]
            self.value = r[3]


class ReadBitResult:
    location = -1
    value = -1

    def __init__(self, res):
        r = unpack('>bbLb', res[:7])
        self.location = r[2]
        self.value = r[3]


class TurnOnResult:
    value = -1

    def __init__(self, res):
        r = unpack('>bbb', res[:3])
        self.value = r[2]


class TurnOffResult:
    value = -1

    def __init__(self, res):
        r = unpack('>bbb', res[:3])
        self.value = r[2]


class WritePageResult:
    page = -1
    bytes = []

    def __init__(self, res):
        r = unpack('>bbH BBBB BBBB BBBB BBBB BBBB BBBB BBBB BBBB', res)
        self.page = r[2]
        self.bytes = r[3:36]

    # def __init__(self, res):
    #     r = unpack('>bbH', res[:4])
    #     self.page = r[2]


class WriteByteResult:
    location = -1

    def __init__(self, res, is_sram_23lc1024=True):
        if is_sram_23lc1024:
            r = unpack('>bbL', res[:6])
            self.location = r[2]
        else:
            r = unpack('>bbH', res[:4])
            self.location = r[2]