require "dsa/lua/stdio"

---C-style 0-indexed array as a C++ style template container.
---Any type is valid, except for `nil` due to how Lua does value lookup.
---@class DSA_Array
---@field type type
---@field buffer any[]
---@field index int
DSA_Array = {} 
DSA_Array.mt = {} ---@type metatable
DSA_Array.__impl = {} -- internal implementation stuff

function DSA_Array:new(typename) ---@param typename type
    local inst = setmetatable({ ---@type DSA_Array
        type = typename,
        buffer = {},
        index = -1, -- Needed for correct first push_back
    }, DSA_Array.mt)
    return inst
end

function DSA_Array:assign(entry, ...) -- Directly reset the internal buffer.
    assert(entry ~= nil, "Need at least 1 argument to infer the type of!")
    local inst = self
    if inst == DSA_Array then -- Is main class so need to create a new instance
        inst = DSA_Array:new(type(entry))
    end
    inst.buffer = {[0] = entry, ...} -- old table is thrown away :(
    inst.index = #inst.buffer
    return inst:validate()
end

function DSA_Array:validate() -- Check if all array elements are the same type.
    for _, v in self:iterate("forward") do
        local a, b = type(v), self.type
        if a ~= b then -- avoid evaluating all args before error
            error(string.format("Invalid type(v) = %s (self.type = %s)", a, b))
        end
    end
    return self
end

function DSA_Array:length() -- Number of contiguous items currently stored.
    return self.index + 1 
end

function DSA_Array:push_back(entry) -- For performance, type(entry) not checked.
    self.index = self.index + 1
    self.buffer[self.index] = entry
    return self
end

function DSA_Array:pop_back()
    local value = self.buffer[self.index]
    self.buffer[self.index] = nil
    self.index = self.index - 1
    return value
end

function DSA_Array:erase(first, last)
    last = last or (first + 1) -- if none, delete only self.buffer[start]
    local length = self.index
    local erased = last - first + 1 -- count of erased elements (off by one -_-)
    printf("first=%i,last=%i,length=%i,erased=%i\n", first, last, length, erased)
    for i = first, last do
        self.buffer[i] = nil
    end
    if last < length then -- Fix array in place if gaps were made.
        local ii = first -- Index range starting with first gap
        -- Index range starting with first non-gap
        for i in self:iterate("forward", last + 1) do 
            self.buffer[i], self.buffer[ii] = self.buffer[ii], self.buffer[i]
            ii = ii + 1
        end
    end
    self.index = length - erased
    return self
end

---@alias iter_modes
---|> "forward"
---|  "reverse"

function DSA_Array:iterate(mode, from) ---@param mode iter_modes?
    mode = mode or "forward"
    -- Our starting index - 1, is incremented by first call to iterator fn.
    ---@type int
    from = (from and from - 1) or (self.buffer[0] ~= nil and -1) or 0 
    local fns = DSA_Array.__impl.iter_fns
    local control = (mode == "forward" and from) or (self.index + 1)
    return fns[mode], self.buffer, control
end

---@alias sort_modes
---|> "ascending"
---|  "descending"

function DSA_Array:sort(mode) ---@param mode (sort_modes|sort_fn)?
    local buffer = self.buffer
    local fns = DSA_Array.__impl.sort_fns
    local compare_fn = (type(mode) == "function" and mode) or fns[mode or "ascending"]
    for i in self:iterate() do -- Selection sort
        local marked = i
        for ii in self:iterate("forward", i + 1) do 
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
