#!/usr/bin/lua5.3

require 'autoangel'

function analyse_sets(s1, s2, cb_1, cb_2, cb, proj)
    local i, j = 1, 1
    while i < #s1 and j < #s2 do
        local a, b = s1[i], s2[j]
        local ida, idb = proj(a), proj(b)
        if ida < idb then
            cb_1(a)
            i = i + 1
        elseif idb < ida then
            cb_2(b)
            j = j + 1
        else
            cb(a, b)
            i = i + 1
            j = j + 1
        end
    end

    while i < #s1 do
        cb_1(s1[i])
        i = i + 1
    end

    while j < #s2 do
        cb_2(s2[j])
        j = j + 1
    end
end

function format_value(val)
    if type(val) == 'number' and val ~= math.floor(val) then
        return string.format('%.4f', val)
    else
        return val
    end
end

function print_diff(edata1, edata2)
    assert(edata1.version == edata2.version)

    for _, list1 in pairs(edata1) do
        local list2 = edata2[list1.type]

        if tonumber(list1.type) ~= 59 then
            local extract_ids = function (l)
                local r = {}
                for _, e in pairs(l) do
                    table.insert(r, e)
                end
                table.sort(r, function (a, b) return a.ID < b.ID end)
                return r
            end

            ids1 = extract_ids(list1)
            ids2 = extract_ids(list2)

            only1_str = ''
            only2_str = ''
            common_str = ''
            local on_removed = function (e) only1_str = string.format('%s\t\t[%d] %s\n', only1_str, e.ID, e.Name) end
            local on_added = function (e) only2_str = string.format('%s\t\t[%d] %s\n', only2_str, e.ID, e.Name) end
            local on_change = function (e1, e2)
                local_str = ''
                for k, v1 in pairs(e1) do
                    local v2 = e2[k]
                    if v1 ~= v2 then
                        local_str = string.format('%s\t\t\t%s: %s -> %s\n', local_str, k, format_value(v1), format_value(v2))
                    end
                end

                if #local_str > 0 then
                    local_str = string.format('\t\t[%d] %s:\n%s', e1.ID, e1.Name, local_str)
                end

                common_str = common_str .. local_str
            end

            analyse_sets(ids1, ids2, on_removed, on_added, on_change, function (e) return e.ID end)

            if #only1_str > 0 then
                only1_str = '\tRemoved:\n' .. only1_str
            end

            if #only2_str > 0 then
                only2_str = '\tAdded:\n' .. only2_str
            end

            if #common_str > 0 then
                common_str = '\tChanged:\n' .. common_str
            end

            total_str = only1_str .. only2_str .. common_str
            if #total_str > 0 then
                print(string.format('<- List #%d ->', tonumber(list1.type)))
                io.write(total_str) -- no newline
            end
        end
    end
end

function main()
    if #arg ~= 3 then
        print(string.format('Usage: %s elements1.data elements2.data configs', arg[0]))
        os.exit(1)
    end

    edata1 = elements.data(arg[1])
    edata2 = elements.data(arg[2])

    conf = elements.load_cfgs(arg[3])

    edata1:load(conf)
    edata2:load(conf)

    print_diff(edata1, edata2)
end

main()
