require "dsa/lua/stdio"

---C-style 0-indexed array as a C++ style template container.
---Any type is valid, except for `nil` due to how Lua does value lookup.
---@class DSA_Array
---@field type type
---@field buffer any[]
---@field length int
DSA_Array = {} 
DSA_Array.mt = {} ---@type metatable
DSA_Array.__impl = {} -- internal implementation stuff

function DSA_Array:new(entry, ...)  ---@return DSA_Array
    local list = {[0] = entry, ...}
    local inst = { ---@type DSA_Array
        type = type(entry),
        buffer = list,
        length = #list
    } 
    for _, v in DSA_Array.iterate(inst) do
        assert(type(v) == inst.type, "array elements must be of the same type!")
    end
    return setmetatable(inst, DSA_Array.mt) 
end

function DSA_Array:push_back(entry)
    self.length = self.length + 1
    self.buffer[self.length] = entry
    return self
end

function DSA_Array:pop_back()
    local value = self.buffer[self.length]
    self.buffer[self.length] = nil
    self.length = self.length - 1
    return value
end

function DSA_Array:erase(first, last)
    last = last or (first + 1) -- if none, delete only self.buffer[start]
    local length = self.length
    local erased = last - first
    for i = first, last do
        self.buffer[i] = nil
    end
    if last < length then -- Fix array in place if gaps were made.
        local ii = first -- Index range starting with first gap
        for i = last + 1, length do -- Index range starting with first non-gap
            self.buffer[i], self.buffer[ii] = self.buffer[ii], self.buffer[i]
            ii = ii + 1
        end
    end
    self.length = length - erased
    return self
end

---@alias iter_modes
---|> "forward"
---|  "reverse"

function DSA_Array:iterate(mode) ---@param mode iter_modes?
    mode = mode or "forward"
    local fns = DSA_Array.__impl.iter_fns
    local control = (mode == "forward" and -1) or (self.length + 1)
    return fns[mode], self.buffer, control
end

---@alias sort_modes
---|> "ascending"
---|  "descending"

function DSA_Array:sort(mode) ---@param mode (sort_modes|sort_fn)?
    local buffer = self.buffer
    local limit = self.length - 1 -- avoid indexing into buffer[length + 1]
    local fns = DSA_Array.__impl.sort_fns
    local compare_fn = (type(mode) == "function" and mode) or fns[mode or "ascending"]
    for i = 0, limit do -- selection sort cuz its easy and can be done in-place
        local marked = i
        for ii = i + 1, limit do
            if compare_fn(buffer[ii], buffer[marked]) then
                marked = ii
            end
        end
        buffer[i], buffer[marked] = buffer[marked], buffer[i]
    end
    return self
end

--*- INTERNALS --------------------------------------------------------------*--
-- Keys are "hashed" or "calculated" at runtime. So it's legal to access keys
-- that are declared after a function definition.
-- 
-- e.g. Array:iterate refers to Array.__impl, which is only declared below.
--*--------------------------------------------------------------------------*--

---@generic T
---@alias iter_fn fun(list: T[], index: int): int?, T?
---@alias sort_fn fun(lhs: T, rhs: T): bool

function DSA_Array.mt.__index(tbl, key) -- nil array index also considered success
    return (type(key) == "number" and tbl.buffer[key]) or DSA_Array[key]
end

function DSA_Array.mt.__newindex(t, k, v) -- https://www.lua.org/pil/13.4.5.html
    error("Array: attempt to modify the class itself from an instance")
end

function DSA_Array.__impl.iter_factory(x_axis) ---@param x_axis -1|1
    return function(list, index) ---@type iter_fn
        index = index + x_axis
        local value = list[index]
        if value then
            return index, value
        end
    end
end

DSA_Array.__impl.iter_fns = { ---@type Dictionary<iter_fn>
    forward = DSA_Array.__impl.iter_factory(1),
    reverse = DSA_Array.__impl.iter_factory(-1),
}

DSA_Array.__impl.sort_fns = { ---@type Dictionary<sort_fn>
    ascending = function(lhs, rhs) -- if true lhs will be first in the array
        return lhs < rhs
    end,
    descending = function(lhs, rhs) 
        return lhs > rhs
    end,
}
