#!/usr/bin/lua5.3

require 'autoangel'
local base64 = require 'base64' -- sudo luarocks install lbase64

URL_TEMPLATE_PATH = 'https://cp.himerapw.net/img/ico_items/%s.jpg'

function make_generator(f)
    co = coroutine.create(f)
    return function()
        local code, res = coroutine.resume(co)
        return res
    end
end

function find_essence_objects(edata, id)
    return make_generator(function()
        for _, list in pairs(edata) do
            if list.space == elements.space_id.essence then
                for _, e in pairs(list) do
                    if e.ID == id then
                        coroutine.yield(e)
                    end
                end
            end
        end
    end)
end

function get_essence_name(edata, id)
    for item in find_essence_objects(edata, id) do
        if item.Name then
            return item.Name
        end
    end
end

function gen_image_url(path)
    path = path:lower()
    b = base64.encode(path):gsub('/', '~')
    return string.format(URL_TEMPLATE_PATH, b)
end

function print_bb_chest_loot(edata, chest_id)
    print(string.format('Дроп с %s:', get_essence_name(edata, chest_id)))

    obj = find_essence_objects(edata, chest_id)()
    for i = 1, 16 do
        drop_id = obj[string.format('materials_%d_id', i)]
        if drop_id > 0 then
            drop_obj = find_essence_objects(edata, drop_id)()

            icon = drop_obj.file_icon
            icon = icon:sub(#icon - icon:reverse():find('\\') + 2)

            name = drop_obj.Name
            if name:sub(1, 1) == '^' then
                name = string.format('[color="#%s"]%s[/color]', name:sub(2, 8), name:sub(8))
            end

            print(string.format('[img]%s[/img] %s', gen_image_url(icon), name))
        end
    end
end

function main()
    if #arg ~= 3 then
        print(string.format('Usage: %s elements.data configs chest_id', arg[0]))
        os.exit(1)
    end

    edata = elements.data(arg[1])
    conf = elements.load_cfgs(arg[2])
    edata:load(conf)

    chest_id = tonumber(arg[3])

    print_bb_chest_loot(edata, chest_id)
end

main()
