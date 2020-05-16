#ifndef P2P_P2PEXCEPTION_H
#define P2P_P2PEXCEPTION_H

#include <string>

namespace p2p {

    class p2pException : public std::runtime_error {
    private:
        const char *file;
        int line;
        const char *info;

    public:
        p2pException(const char *info_, const char *file_, int line_) :
                file(file_),
                line(line_),
                info(info_),
                std::runtime_error((std::string)info_ +
                " File: " + std::string(file_) +
                " Line: " + std::to_string(line_)) {
        }

        [[nodiscard]] const char *getFile() const { return file; }

        [[nodiscard]] int getLine() const { return line; }

        [[nodiscard]] const char *getInfo() const { return info; }
    };
}

#define throw_p2p_exception(what) throw p2pException(what, __FILENAME__, __LINE__)

#endif
