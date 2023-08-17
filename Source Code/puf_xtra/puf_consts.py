# Adds immutable constants for software-based-PUF,
# https://github.com/Tribler/software-based-PUF
# Copyright (C) 2023 myndcryme
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from collections import namedtuple

Mode = namedtuple('Constants', ['DEFAULT', 'MONO', 'PARALLEL'])
mode = Mode(0, 0, 1)

Device = namedtuple('Constants', ['GENUINE_MEGA2560_R3'])
device = Device('2341:0042')        # VID:PID

# any attempts to re-assign a value fails and raises an AttributeError
