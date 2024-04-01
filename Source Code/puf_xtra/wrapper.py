# This file contains wrappers of select functions and is part of
# software-based-PUF, https://github.com/Tribler/software-based-PUF
# Copyright (C) 2024 myndcryme
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

import sys
import os

def sys_path_append(paths):
    """
    Wrapper for sys.path.append that includes checks.  Accepts single path
    string or a list of strings to temporarily append to system path
    :param paths: string or list of strings containing paths to add
    :return: 0 if all succeed, 1 on error
    """
    if isinstance(paths, str):
        paths = [paths]     # convert string to list
    for p in paths:
        if not os.path.exists(p):    # check for non-existent path
            return 1
        path = os.path.abspath(p)
        for x in sys.path:
            x = os.path.abspath(x)
            if path in (x, x+os.sep):     # already in sys.path
                return 0
        sys.path.append(path)
        print(path, "added to sys.path (temporary)")
        return 0
    return 1
