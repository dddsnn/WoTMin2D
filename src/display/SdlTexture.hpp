#ifndef SDLTEXTURE_HPP
#define SDLTEXTURE_HPP

#include "SdlException.hpp"

#include <SDL.h>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <array>

namespace wotmin2d {

class SdlTexture {
    public:
    using Color = std::array<std::uint8_t, 3>;
    SdlTexture(SDL_Renderer* renderer, unsigned int texture_width,
               unsigned int texture_height);
    SdlTexture(const SdlTexture& other) = delete;
    SdlTexture& operator=(SdlTexture other) = delete;
    ~SdlTexture();
    void lockForWriting();
    void unlock();
    bool isLocked() const;
    void setPixel(unsigned int x, unsigned int y, const Color& color);
    void setPixelIndex(unsigned int index, const Color& color);
    void setPixelRange(unsigned int start, unsigned int end,
                       const Color& color);
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    SDL_Texture* getTexture() const;
    private:
    std::uint32_t findPixelFormat(SDL_Renderer* renderer);
    const SDL_Renderer* const renderer;
    const unsigned int texture_width;
    const unsigned int texture_height;
    SDL_Texture* texture;
    SDL_PixelFormat* pixel_format;
    void* pixels;
    int pitch;
};

}

#endif
