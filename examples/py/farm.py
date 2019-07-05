#!/usr/bin/python3

from autoangel import *
import random
import collections
import sys

def rand_select(l):
    r = random.random()
    for i in range(len(l)):
        if r < l[i]:
            return i
        r -= l[i]
    assert False

# returns item list
def generate_item_from_monster(edata, id: int):
    mon = next(monster for monster in edata[38] if monster.ID == id)

    probability_drop_num = [getattr(mon, 'probability_drop_num{}'.format(i)) for i in range(4)]
    drop_matters_probability = [getattr(mon, 'drop_matters_{}_probability'.format(i + 1)) for i in range(32)]

    output = collections.defaultdict(int)

    mcount = mon.drop_times
    for j in range(mcount):
        drop_num = rand_select(probability_drop_num)
        for i in range(drop_num):
            index = rand_select(drop_matters_probability)
            if j > 0 and index >= 16:
                continue
            drop_id = getattr(mon, 'drop_matters_{}_id'.format(index + 1))
            output[drop_id] += 1

    return output


def generate_item_mean(edata, id: int, count: int):
    d = collections.defaultdict(int)
    for _ in range(count):
        r = generate_item_from_monster(edata, id)
        for k, v in r.items():
            d[k] += v
    for k in d:
        d[k] /= count
    return d


def find_essence_objects(edata, id: int):
    for list in edata:
        if list.space == elements.space_id.essence:
            for e in list:
                if e.ID == id:
                    yield e


def get_essence_name(edata, id: int):
    for item in find_essence_objects(edata, id):
        if item.Name:
            return item.Name


if __name__ == "__main__":
    if len(sys.argv) not in (4, 5):
        print('Usage: {} elements.data configs mob_id [try_count]'.format(sys.argv[0]))
        exit(1)

    edata = elements.data(sys.argv[1])
    conf = elements.load_cfgs(sys.argv[2])
    edata.load(conf)

    mob_id = int(sys.argv[3])
    try_count = int(sys.argv[4]) if len(sys.argv) == 5 else 100

    loot = generate_item_mean(edata, mob_id, try_count)
    print('Лут с {} (усреднение по {} попыткам):'.format(get_essence_name(edata, mob_id), try_count))
    for id, prob in loot.items():
        print('[{}] {} - {} шт.'.format(id, get_essence_name(edata, id), round(prob, 2)))
