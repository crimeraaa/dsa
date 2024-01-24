local function print_format(fmt, ...)
    io.stdout:write(fmt:format(...))
end

local function print_result(text, fn)
    print_format("-*- BEGIN:    %s\n", text)
    fn()
    print_format("-*- END:      %s\n\n", text)
end

local function make_fibonacci_sequence(reps)
    local f1, f2 = 0, 1
    local fn = f1 + f2 -- `f@n = f@n-1 + f@n-2`
    local list = {}
    for i = 1, reps do
        list[i] = f1
        f1, f2 = f2, fn -- must be updated before f@n
        fn = f1 + f2
    end
    return list
end
fibonacci_sequence = make_fibonacci_sequence(8)

--[[--------------------------------------------------------------------------*-
FIGURE 1.1: STATEFUL ITERATORS - https://www.lua.org/pil/7.1.html

A Lua iterator is just a function. Everytime it's called it returns the next
element. We first create a *factory* which sets up the closure variables.

The closure variables are just the input table itself, the table length 
and an iterator index.

After setting up said closure variables we can create the closure function.
It will modify/mutate the index and try to return the value at that table index
every time it is called.

-*--------------------------------------------------------------------------*-]]

function list_iter(tbl)
    local i = 0
    local n = #tbl
    return function()
        i = i + 1
        if i <= n then
            return tbl[i]
        end
    end
end

--[[--------------------------------------------------------------------------*-
FIGURE 1.2: GENERIC FOR LOOP

The generic for loop takes the following form:

for <control>, [...] in <iteratorfn> do 
    <body>
end

<control>:      The first return value of <iteratorfn>, the iterator variable.
[...]:          Other optional return values from <iteratorfn>.
<iteratorfn>:   A closure function created by an iterator factory.
<body>:         What to do with the value/s.

One powerful feature of the generic for is that it keeps state by itself, so
we really don't need stateful iterators. We'll learn about statelessness later.

Everytime the iterator function is called, it's used to update the iterator 
variable. This is why <control> is so important! Once it's nil, the loop ends.
The power of <iteratorfn> is that you can use different ones. You can create
one to return a key-value pair (ala `pairs`), an index-value pair (ala `ipairs`)
or a key1-value1-key2-value2 tuple, etc. 
-*--------------------------------------------------------------------------*-]]

print_result("STATEFUL ITERATOR", function() 
    for element in list_iter(fibonacci_sequence) do
        print(element)
    end
end)

--[[--------------------------------------------------------------------------*-
FIGURE 1.3: GENERIC FOR LOOP SEMANTICS - https://www.lua.org/pil/7.2.html

Using closures frequently is not desireable for some cases since the external 
local variables in the closure aren't destroyed until the function itself is.
This puts a little bit of strain on the Lua garbage collector ever so slowly.

We can instead use the generic for loop itself to keep track of state.
This is because the loop keeps track of 3 values:
1. Iterator function
2. Invariant state
3. Control variable

Such as in the form:

for <varlist> in <explist> do
    ...
end

<varlist>:  1/more comma separated variable names/identifiers.
            *   The first element is always the control variable. 
            *   It is used to:
                i.  Iterate through the list
                ii. Determine if we should stop iterating
<explist>:  Usually the call to an iterator factory.
            * The iterator factory will in turn:
                i.  Return a function that returns 1/more values.

See the following code:

for key, value in pairs(tbl) do
    print(key, value)
end

<varlist>:  variables `key` and `value`. `key` is our loop control variable.
            *   One `key` is `nil`, the loop breaks and ends.
<explist>:  The return value/s of the iterator factory `pairs(tbl)`.
            *   The return value/s of `pairs(tbl)` are:
                i.  Iterator function
                ii. Invariant state
                iii.Initial value for control variable
                    * This will be the first value `key` starts with.
            
The generic for loop first evalues everything after the `in` keyword.
After that, the loop will constantly evaluate only 2 things:
1.  Invariant state  (such as the table itself)
    * Ideally this should not change while in the loop, hence "invariant".
    * In reality though nothing is stopping you from doing so...
2.  Control variable (as updated via iterator function)
    * If at any point this is `nil`, the loop terminates. 
    * This is always the first return value of the iterator function.

Upon every call to the iterator function, everything in <varlist> is assigned.
-*--------------------------------------------------------------------------*-]]

--[[--------------------------------------------------------------------------*-
FIGURE 1.5: STATELESS ITERATORS - https://www.lua.org/pil/7.3.html

With the generic for loop semantics in mind, we can creates iterators that are
not closures.

First we create a function that takes exactly 2 arguments:
1.  The table itself
2.  Some requested key/index
It will then return:
1.  The new index
2.  The value at this index in the table

Then, for our implementation of `ipairs`, it will take exactly 1 argument:
1.  The table itself
Then it will return:
1.  <iteratorfn>:   The iterator function `my_iter`
2.  <invariant>:    The table itself
3.  <controlvar>:   Initial value for control variable

For example, in `my_ipairs`, note how our control variable starts at 0.
Immediately after being initialized, the for loop begins. Here the iterator
function is called.

So since our control is 0, the first call will be `my_iter(array, 0)`.
Inside, `index` is updated to be `index + 1` or just `1`. 
The return value will be `1, array[1]`.

Keep going until we hit `nil`!
------------------------------------------------------------------------------]]

function my_iter(array, index)
    index = index + 1
    local value = array[index]
    if value ~= nil then
        return index, value
    end
end

function my_ipairs(array)
    return my_iter, array, 0
end

print_result("STATELESS ITERATOR", function() 
    for i, v in my_ipairs(fibonacci_sequence) do
        print(i, v) 
    end
end)

--[[--------------------------------------------------------------------------*-
FIGURE 1.6 - 0-BASED STATELESS ITERATORS

This is now my own experimentation to replicate C-style 0-based indexing.
-*--------------------------------------------------------------------------*-]]

-- the concept: #array will only count array[1]...array[n], not array[0]!

local zerobased = {[0] = 2, 3, 5, 7, 11, 13, 21}
print("#zerobased is " .. #zerobased)

-- But looping is still ok if we start at 0, since the last index is correct.

print_result("0-BASED NUMERIC FOR LOOP", function()
    for i = 0, #zerobased, 1 do
        print_format("zerobased[%i] = %i\n", i, zerobased[i])
    end
end)

---@generic T: table, V
---@param list T
---@param index integer
---@return integer?
---@return V?
local function zero_iter(list, index)
    index = index + 1
    local value = list[index] 
    if value then
        return index, value 
    end
end

---@generic T: table, V
---@param target T
---@return fun(list: V[], index: integer): integer, V
---@return T
---@return integer
local function zero_ipairs(target)
    return zero_iter, target, -1 -- need to start at -1 so first call gives us 0
end

print_result("0-BASED GENERIC FOR LOOP", function() 
    for i, v in zero_ipairs(zerobased) do
        print_format("zerobased[%i] = %i\n", i, v)
    end
end)

---@generic T
---@param target T[]
---@return fun(): T?
local function values(target) -- Problem: requires a closure!
    local index = -1
    local length = #target
    return function()
        index = index + 1
        if index <= length then
            return target[index]
        end
    end
end

print_result("0-BASED FOREACH LOOP", function()
    for v in values(zerobased) do
        print_format("%i\n", v)
    end
end)
