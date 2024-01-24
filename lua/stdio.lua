-- Print a C-style format string. No newlines are appended.
---@param fmt str
---@param ... str|num
function printf(fmt, ...)
    io.stdout:write(fmt:format(...))
end

-- Write a string to the standard output stream. No newlines are appended.
---@param str str
function puts(str)
    io.stdout:write(str)
end

-- Write a C-style format string to `stream`. No newlines are appended.
---@param stream file*
---@param fmt str
---@param ... str|num
function fprintf(stream, fmt, ...)
    stream:write(fmt:format(...))
end

-- Write a string to the specified file stream. No newlines are appended.
---@param stream file*
---@param str str
function fputs(stream, str)
    stream:write(str)
end
