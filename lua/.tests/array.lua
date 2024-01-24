---@note Ensure absolute path of `repos/dsa/lua` is in `LUA_PATH`!
---export LUA_PATH="?.lua;?/init.lua;~/repos/?.lua"

require "dsa/lua/array"
require "dsa/lua/stdio"
require "dsa/lua/system"

fputs(io.stderr, "hello from stderr :)\n")
printf("script path: %s\n", os.script_path())

local function loopy(array) ---@param array DSA_Array
    printf("array: [length=%i, type=\"%s\"]\n", array.length, array.type)
    for i, v in array:iterate() do
        printf("array.buffer[%i] = %i\n", i, v)
    end
    print()
end

local array = DSA_Array:new(2, 3, 5, 7, 11, 13) 
loopy(array)
loopy(array:push_back(100):push_back(49))
loopy(array:erase(1, 3))
loopy(array:sort("descending"))
