#!/usr/bin/python3

from autoangel import *
import sys
import base64

URL_TEMPLATE_PATH = 'https://cp.himerapw.net/img/ico_items/{path}.jpg'


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


def gen_image_url(path: str):
    path = path.lower().encode('utf8')
    b = base64.b64encode(path).decode('ascii').replace('/', '~')
    return URL_TEMPLATE_PATH.format(path=b)


def print_bb_chest_loot(edata, chest_id: int):
    print('Дроп с {}:'.format(get_essence_name(edata, chest_id)))
    obj = next(find_essence_objects(edata, chest_id))
    for i in range(16):
        drop_id = getattr(obj, 'materials_{}_id'.format(i + 1))
        if not drop_id:
            continue

        drop_obj = next(find_essence_objects(edata, drop_id))

        icon = drop_obj.file_icon
        icon = icon[icon.rfind('\\')+1:]

        name = drop_obj.Name
        if name[0] == '^':
            name = '[color="#{}"]{}[/color]'.format(name[1:7], name[7:])

        print('[img]{}[/img] {}'.format(gen_image_url(icon), name))


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print('Usage: {} elements.data configs chest_id'.format(sys.argv[0]))
        exit(1)

    edata = elements.data(sys.argv[1])
    conf = elements.load_cfgs(sys.argv[2])
    edata.load(conf)

    chest_id = int(sys.argv[3])

    print_bb_chest_loot(edata, chest_id)


