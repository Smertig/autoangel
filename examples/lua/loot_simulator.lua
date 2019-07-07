#!/usr/bin/lua5.3

require 'autoangel'

function rand_select(l)
    r = math.random()
    for i = 1, #l do
        if r < l[i] then
            return i
        end
        r = r - l[i]
    end
    assert(false)
end

-- returns item list
function generate_item_from_monster(edata, id)
    local mon
    for _, monster in pairs(edata[38]) do
        if monster.ID == id then
            mon = monster
            break
        end
    end

    local probability_drop_num = {}
    for i = 1, 4 do
        probability_drop_num[i] = mon[string.format('probability_drop_num%d', i - 1)]
    end

    local drop_matters_probability = {}
    for i = 1, 32 do
        drop_matters_probability[i] = mon[string.format('drop_matters_%d_probability', i)]
    end

    local output = {}

    local mcount = mon.drop_times
    for j = 1, mcount do
        local drop_num = rand_select(probability_drop_num) - 1
        for i = 1, drop_num do
            local index = rand_select(drop_matters_probability)
            if j == 0 or index <= 16 then
                local drop_id = mon[string.format('drop_matters_%d_id', index)]
                output[drop_id] = (output[drop_id] or 0) + 1
            end
        end
    end

    return output
end

function generate_item_mean(edata, id, count)
    local d = {}
    for _ = 1, count do
        r = generate_item_from_monster(edata, id)
        for k, v in pairs(r) do
            d[k] = (d[k] or 0) + v
        end
    end

    for k, _ in pairs(d) do
        d[k] = d[k] / count
    end

    return d
end

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

function main()
    if #arg ~= 3 and #arg ~= 4 then
        print(string.format('Usage: %s elements.data configs mob_id [try_count]', arg[0]))
        os.exit(1)
    end

    edata = elements.data(arg[1])
    conf = elements.load_cfgs(arg[2])
    edata:load(conf)

    mob_id = tonumber(arg[3])
    try_count = tonumber(arg[4] or 100)

    loot = generate_item_mean(edata, mob_id, try_count)
    print(string.format('Лут с %s (усреднение по %d попыткам)', get_essence_name(edata, mob_id), try_count))
    for id, prob in pairs(loot) do
        print(string.format('[%d] %s - %.2f шт.', id, get_essence_name(edata, id), prob))
    end
end

main()
