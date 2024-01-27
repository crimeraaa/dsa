#pragma once

#include <cstdio>
/**
 * @brief   Need in order to properly get length of formatted `wchar_t` strings.
 *          By using a class with constructors and destructors, we ensure that
 *          the file is opened before entry and closed after main exits.
 *
 * @note    This is only as issue on Linux, where the call:
 *          `int n = vswprintf(nullptr, 0, fmts, args)` returns -1.
 *          On Windows I *believe* such a call is valid.
 */
class FileHandle {
private:
    std::FILE *m_pfile = nullptr;
    const char *m_pname = nullptr;
public:
    FileHandle(const char *filename) 
        : m_pfile{std::fopen(filename, "wb")}
        , m_pname{filename}
    {
        if (m_pfile == nullptr) {
            perror("FileHandle() failed to open filename");
        }
    }  
    
    ~FileHandle() 
    {
        std::fclose(m_pfile);
    }
    
    std::FILE *get_fileptr() 
    {
        return m_pfile;
    }
};
