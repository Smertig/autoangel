#!/usr/bin/lua5.3

require 'autoangel'

function analyse_sets(s1, s2, cb_1, cb_2, cb, proj)
    local i, j = 1, 1
    while i <= #s1 and j <= #s2 do
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

    while i <= #s1 do
        cb_1(s1[i])
        i = i + 1
    end

    while j <= #s2 do
        cb_2(s2[j])
        j = j + 1
    end
end

function calc_diff(pack1, pack2)
    only1_str = ''
    only2_str = ''
    common_str = ''
    local on_removed = function (file) only1_str = string.format('%s\t[%s] %d bytes\n', only1_str, file, #pack1:read(file)) end
    local on_added = function (file) only2_str = string.format('%s\t[%s] %d bytes\n', only2_str, file, #pack2:read(file)) end
    local on_change = function (file1, file2)
        content1 = pack1:read(file1)
        content2 = pack2:read(file2)
        if content1 ~= content2 then
            common_str = string.format('%s\t[%s] %d -> %d bytes\n', common_str, file1, #content1, #content2)
        end
    end

    analyse_sets(pack1:find_prefix(''), pack2:find_prefix(''), on_removed, on_added, on_change, function (file) return file end)

    if #only1_str > 0 then
        only1_str = 'Removed:\n' .. only1_str
    end

    if #only2_str > 0 then
        only2_str = 'Added:\n' .. only2_str
    end

    if #common_str > 0 then
        common_str = 'Changed:\n' .. common_str
    end

    return only1_str .. only2_str .. common_str
end

function main()
    if #arg ~= 2 then
        print(string.format('Usage: %s package1.pck package2.pck', arg[0]))
        os.exit(1)
    end

    pack1 = pck.package(arg[1])
    pack2 = pck.package(arg[2])

    pack1:load()
    pack2:load()

    print(calc_diff(pack1, pack2))
end

main()
