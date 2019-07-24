#!/usr/bin/python3

from autoangel import *
import sys

def calc_diff(pack1, pack2):
    files1 = set(pack1.find_prefix(''))
    files2 = set(pack2.find_prefix(''))

    only1 = sorted(files1 - files2)
    only2 = sorted(files2 - files1)
    common = sorted(files1 & files2)

    only1_str = ''
    for file in only1:
        only1_str += '\t[{}] {} bytes\n'.format(file, len(pack1.read(file)))

    only2_str = ''
    for file in only2:
        only2_str += '\t[{}] {} bytes\n'.format(file, len(pack2.read(file)))

    common_str = ''
    for file in common:
        content1 = pack1.read(file)
        content2 = pack2.read(file)
        if content1 != content2:
            common_str += '\t[{}] {} -> {} bytes\n'.format(file, len(content1), len(content2))

    if len(only1) > 0:
        only1_str = 'Removed:\n' + only1_str

    if len(only2) > 0:
        only2_str = 'Added:\n' + only2_str

    if len(common_str) > 0:
        common_str = 'Changed:\n' + common_str

    return only1_str + only2_str + common_str


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print('Usage: {} package1.pck package2.pck'.format(sys.argv[0]))
        exit(1)

    pack1 = pck.package(sys.argv[1])
    pack2 = pck.package(sys.argv[2])

    pack1.load()
    pack2.load()

    print(calc_diff(pack1, pack2))

