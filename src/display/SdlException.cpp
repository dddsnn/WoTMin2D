#include "SdlException.hpp"

namespace wotmin2d {

SdlException::SdlException(const char* what_arg, const char* sdl_error) :
    std::runtime_error(what_arg),
    sdl_error(sdl_error) {
}

const char* SdlException::sdlError() const noexcept {
    return sdl_error;
}

}
