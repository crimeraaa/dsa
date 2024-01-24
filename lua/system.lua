local function get_platform() -- See: https://stackoverflow.com/a/14425862
    return package.config:sub(1, 1) == '\\' and "win" or "unix"
end

os.platform = get_platform()
os.slash = (os.platform == "win" and '\\') or '/'

function os.script_path() -- See: https://stackoverflow.com/a/23535333
    -- 0 = debug.getinfo
    -- 1 = os.script_path
    -- 2 = caller of os.script_path
    local source = debug.getinfo(2, "S").source:sub(2)
    return source:match("(.*[/\\])") or ("." .. os.slash) ---@type str
end

function os.convert_path(path) ---@param path str
    return path:gsub("[/\\]", os.slash)
end

