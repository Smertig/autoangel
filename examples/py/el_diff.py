#!/usr/bin/python3

from autoangel import *
import sys

def calc_diff(edata1, edata2):
    assert(edata1.version == edata2.version)

    output = ''
    for list1, list2 in zip(edata1, edata2):
        if int(list1.type) == 59:
            continue

        m1 = {e.ID: e for e in list1}
        m2 = {e.ID: e for e in list2}

        only1 = sorted(m1.keys() - m2.keys())
        only2 = sorted(m2.keys() - m1.keys())
        common = sorted(m1.keys() & m2.keys())

        only1_str = ''
        for k in only1:
            only1_str += '\t\t[{}] {}\n'.format(k, m1[k].Name)

        only2_str = ''
        for k in only2:
            only2_str += '\t\t[{}] {}\n'.format(k, m2[k].Name)

        common_str = ''
        for id in common:
            e1 = m1[id]
            e2 = m2[id]
            changed_local_str = ''
            for (k1, v1), (k2, v2) in zip(e1, e2):
                if v1 != v2:
                    l = lambda val: val if type(val) != float else '{:.04f}'.format(val)
                    changed_local_str += '\t\t\t{}: {} -> {}\n'.format(k1, l(v1), l(v2))

            if len(changed_local_str) > 0:
                changed_local_str = '\t\t[{}] {}:\n'.format(id, e1.Name) + changed_local_str

            common_str += changed_local_str

        if len(only1) > 0:
            only1_str = '\tRemoved:\n' + only1_str

        if len(only2) > 0:
            only2_str = '\tAdded:\n' + only2_str

        if len(common_str) > 0:
            common_str = '\tChanged:\n' + common_str

        total_str = only1_str + only2_str + common_str
        if len(total_str) > 0:
            output += '<- List #{} ->\n'.format(int(list1.type))
            output += total_str

    return output


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print('Usage: {} elements1.data elements2.data configs'.format(sys.argv[0]))
        exit(1)

    edata1 = elements.data(sys.argv[1])
    edata2 = elements.data(sys.argv[2])

    conf = elements.load_cfgs(sys.argv[3])

    edata1.load(conf)
    edata2.load(conf)

    print(calc_diff(edata1, edata2))

