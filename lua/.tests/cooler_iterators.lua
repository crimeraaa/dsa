require "dsa/lua/stdio"

---@generic T
---@param array Array<T>
---@param index int
---@return int?, T? 
local function basic_iter_fn(array, index)
    index = index + 1
    local value = array[index]
    if value then
        return index, value
    end
end


local compare_fns = { ---@type Dictionary<sort_fn>
    ascending = function(lhs, rhs) 
        return lhs < rhs 
    end,
    descending = function(lhs, rhs)
        return lhs > rhs
    end,
}

local ssort = {} -- selection sort stuff

---@generic T
---@param array Array<T>
---@param initial int?
---@return iter_fn, Array<T>, int
function ssort.iterate(array, initial)
    initial = initial or (array[0] ~= nil and -1) or 0
    return basic_iter_fn, array, initial
end

---@generic T
---@param array Array<T>
---@param mode sort_modes?
---@return Array<T> array
function ssort.alpha(array, mode) 
    local length = #array
    local limit = length - 1 -- save some instructions by not doing very last
    local compare = compare_fns[mode or "ascending"]
    for i = (array[0] and 0) or 1, length do 
        local marked = i
        for ii = i + 1, length do
            if compare(array[ii], array[marked]) then
                marked = ii
            end
        end
        array[i], array[marked] = array[marked], array[i]
    end
    return array
end

---@generic T
---@param array Array<T>
---@param mode sort_modes?
---@return Array<T> array
function ssort.bravo(array, mode)
    local compare = compare_fns[mode or "ascending"]
    for i in ssort.iterate(array) do
        local marked = i
        for ii in ssort.iterate(array, i) do -- NOTE: Don't use i + 1
            if compare(array[ii], array[marked]) then
                marked = ii
            end
        end
        array[i], array[marked] = array[marked], array[i]
    end
    return array
end

---@generic T
---@param array Array<T>
---@return iter_fn, Array<T>, int
local function iter(array)
    -- work with both 0-based and 1-based arrays
    local initial = (array[0] ~= nil and -1) or 0 
    return basic_iter_fn, array, initial
end

local t = {} ---@type Array<int>
local zerobased = true
local start = (zerobased and 0) or 1
local length = 16

-- math.randomseed(123456789119)
math.randomseed(os.time())
for i = start, length do
    t[i] = math.random(1, 0xFF)
end
for i, v in iter(t) do
    printf("t[%i] = %s\n", i, tostring(v))
end
puts('\n')
for i, v in iter(ssort.bravo(t, "ascending")) do
    printf("t[%i] = %i\n", i, v)
end
