#ifndef SDLTEXTURE_HPP
#define SDLTEXTURE_HPP

#include "SdlException.hpp"

#include <SDL.h>
#include <cstdint>
#include <cassert>
#include <algorithm>

namespace wotmin2d {

class SdlTexture {
    public:
    SdlTexture(SDL_Renderer* renderer, unsigned int width,
               unsigned int height);
    SdlTexture(const SdlTexture& other) = delete;
    SdlTexture& operator=(SdlTexture other) = delete;
    ~SdlTexture();
    void lockForWriting();
    void unlock();
    void setPixel(unsigned int index, std::uint8_t red, std::uint8_t green,
                  std::uint8_t blue);
    void setRange(unsigned int start, unsigned int end, std::uint8_t red,
                  std::uint8_t green, std::uint8_t blue);
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    SDL_Texture* getTexture() const;
    private:
    std::uint32_t findPixelFormat(SDL_Renderer* renderer);
    const SDL_Renderer* const renderer;
    const unsigned int width;
    const unsigned int height;
    SDL_Texture* texture;
    SDL_PixelFormat* pixel_format;
    void* pixels;
    int pitch;
};

}

#endif
