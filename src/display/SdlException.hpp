#ifndef SDLEXCEPTION_HPP
#define SDLEXCEPTION_HPP

#include <stdexcept>

namespace wotmin2d {

class SdlException : public std::runtime_error {
    public:
    SdlException(const char* what_arg, const char* sdl_error);
    const char* sdlError() const noexcept;
    private:
    const char* sdl_error;
};

}

#endif
