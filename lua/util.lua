function printf(format, ...)
    io.stdout:write(format:format(...))
end

function printfln(format, ...)
    return printf(format .. '\n', ...)
end

--- https://en.wikipedia.org/wiki/Filter_(higher-order_function)
function filter(src, predicate)
    local filtered = {}
    local count = #filtered
    for _, v in ipairs(src) do
        if predicate(v) then
            count = count + 1
            filtered[count] = v
        end
    end
    return filtered
end

--- https://en.wikipedia.org/wiki/Fold_(higher-order_function)
function reduce(src, operation, initial_value)
    local acc = initial_value or 0
    for _, v in ipairs(src) do
        acc = operation(v, acc)
    end
    return acc
end

--- https://en.wikipedia.org/wiki/Map_(higher-order_function)
function map(src, operation)
    local mapped = {}
    for k, v in pairs(src) do
        mapped[k] = operation(v)
    end
    return mapped
end

function dump_table(t, name)
    print(name or t)
    for k, v in pairs(t) do
        printfln("[%s]: %s", tostring(k), tostring(v))
    end
end
