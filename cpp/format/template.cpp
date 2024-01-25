#include <cstdio>
#include <string>

/**
 * @brief   Need in order to properly get length of formatted `wchar_t` strings.
 *
 * @note    This is only as issue on Linux, where the call:
 *          `int n = vswprintf(nullptr, 0, fmts, args)` returns -1.
 *          On Windows I *believe* such a call is valid.
 */
class NullDevice {
private:
    static constexpr const char *filename = "/dev/null";
    std::FILE *m_pfile = nullptr;
public:
    NullDevice() : m_pfile{std::fopen(filename, "wb")}
    {
        if (m_pfile == nullptr) {
            perror("NullDevice() failed to open /dev/null");
        }
    }  
    
    ~NullDevice() 
    {
        std::fclose(m_pfile);
    }
    
    std::FILE *get_fileptr() 
    {
        return m_pfile;
    }
};

static NullDevice nulldevice;

int main() 
{
    return 0;
}
