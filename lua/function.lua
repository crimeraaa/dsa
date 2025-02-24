--- A type representing mathematical functions and their possible operations.
---@class Function
---@field fn Function.Callback
local Function = {}

---@alias Function.Callback fun(x: number): (y: number)

---@type metatable
local mt = {}

---@param cond boolean
---@param format string
---@param ... any
local function assertf(cond, format, ...)
    if not cond then
        error(format:format(...))
    end
end

---@param fn Function.Callback
local function new(fn)
    ---@type Function
    local self = setmetatable({}, mt)
    local typename = type(fn)
    assertf(typename == "function", "Bad argument #1: Expected 'function', got %s", typename)
    self.fn = fn
    return self
end

---@param f Function
---@param g Function
---@param x number
function Function.compose(f, g, x)
    -- Quicker to call field directly than to invoke metatable lookup
    return f.fn(g.fn(x))
end

---@param self Function
---@param x number
function mt.__call(self, x)
    return self.fn(x)
end

--- METATABLE -------------------------------------------------------------- {{{

mt.__index = Function

---@param f Function
---@param g Function
---@return Function
function mt.__add(f, g)
    return new(function(x) return f.fn(x) + g.fn(x) end)
end

---@param f Function
---@param g Function
---@return Function
function mt.__sub(f, g)
    return new(function(x) return f.fn(x) - g.fn(x) end)
end

---@param f Function
---@param g Function
---@return Function
function mt.__mul(f, g)
    return new(function(x) return f.fn(x) * g.fn(x) end)
end

---@param f Function
---@param g Function
---@return Function
function mt.__div(f, g)
    return new(function(x) return f.fn(x) / g.fn(x) end)
end

--- }}} ------------------------------------------------------------------------

return setmetatable(Function, {
    --- Allow for class-style construction.
    ---@param self Function The `Function` table itself.
    ---@param fn Function.Callback
    __call = function(self, fn)
        return new(fn)
    end,
})
