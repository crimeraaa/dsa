---@note Ensure absolute path of `repos/dsa/lua` is in `LUA_PATH`!
---export LUA_PATH="?.lua;?/init.lua;~/repos/?.lua"

require "dsa/lua/array"
require "dsa/lua/stdio"
require "dsa/lua/system"

fputs(io.stderr, "hello from stderr :)\n")
printf("script path: %s\n", os.script_path())

local function loopy(array) ---@param array DSA_Array
    printf("array: [index=%i, length=%i, type=\"%s\"]\n", array.index, array:length(), array.type)
    for i, v in array:iterate() do
        printf("array.buffer[%i] = %i\n", i, v)
    end
    print()
end

local function try_catch(callback, ...)
    print("-*- BEGIN:  TEST -*-")
    local success, errmsg = pcall(callback, ...) -- protected call
    if not success then -- sort of like try-catch block
        print(errmsg)
    end
    print("-*- END:    TEST -*-\n")
end

local array = DSA_Array:new("number")
try_catch(function() 
    local limit = math.random(1, 0xFF)
    for i = 1, 16 do
        array:push_back(math.random(1, limit))
    end
    loopy(array)
    loopy(array:erase(0, 2))
    loopy(array:erase(1, 3))
    loopy(array:sort("ascending"))
    loopy(array:sort("descending"))
end)

try_catch(function()
    loopy(array:push_back(45000):push_back(tonumber("1101", 2)))
    loopy(array:erase(2,5))
    loopy(array:push_back("nope"):validate())
    loopy(array:push_back(tonumber("FEEDBEEF", 16)))
end)
