---@class Set
---@field data {[any]: boolean}
local Set = {}

---@type metatable
local mt = {}

local function new_set(array)
    ---@type Set
    local self = setmetatable({}, mt)
    local data = {}
    for _, value in pairs(array or {}) do
        data[value] = true
    end
    self.data = data
    return self
end

local function is_set(object)
    return getmetatable(object) == mt
end

--- METHODS ---------------------------------------------------------------- {{{

function Set:contains(key)
    return self.data[key] ~= nil
end

function Set:insert(key)
    self.data[key] = true
end

function Set:remove(key)
    self.data[key] = nil
end

local function _copy_set_or_array(dst, src)
    if is_set(src) then
        for key in pairs(src.data) do
            dst:insert(key)
        end
    else
        for _, value in pairs(src) do
            dst:insert(value)
        end
    end
end

--- Create a new set containing both members of `self` and `other`.
function Set:union(other)
    local union = new_set()
    _copy_set_or_array(union, self)
    _copy_set_or_array(union, other)
    return union
end

--- Create a new set containing the members of `self` that are also in `other`.
function Set:intersection(other)
    local intersect = new_set()

    -- We require `other` to be a `Set` because we need to use its hash table.
    if not is_set(other) then other = new_set(other) end

    if is_set(self) then
        for key in pairs(self.data) do
            if other.data[key] then intersect:insert(key) end
        end
    -- May occur when calling form `Set.intersection({1, 2, 3}, {2, 3, 4})`
    else
        for _, value in pairs(self) do
            if other.data[value] then intersect:insert(value) end
        end
    end
    return intersect
end

-- Create a new set containing the members of `self` that are NOT in `other`.
function Set:difference(other)
    local difference = new_set()

    if not is_set(other) then other = new_set(other) end

    if is_set(self) then
        for key in pairs(self.data) do
            if not other.data[key] then difference:insert(key) end
        end
    else
        for _, value in pairs(self) do
            if not other.data[value] then difference:insert(value) end
        end
    end
    return difference
end

--- }}} ------------------------------------------------------------------------

--- METATABLE -------------------------------------------------------------- {{{

mt.__index = Set
mt.__add = Set.union
mt.__mul = Set.intersection
mt.__sub = Set.difference

---@param self Set
function mt.__tostring(self)
    local buffer = {}
    local index = #buffer
    for key in pairs(self.data) do
        index = index + 1
        buffer[index] = tostring(key)
    end
    return '{' .. table.concat(buffer, ", ") .. '}'
end

--- }}} ------------------------------------------------------------------------

return setmetatable(Set, {
    __call = function(self, set)
        return new_set(set)
    end,
})
