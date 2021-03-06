#include "SdlTexture.hpp"

namespace wotmin2d {

SdlTexture::SdlTexture(SDL_Renderer* renderer, unsigned int texture_width,
                       unsigned int texture_height) :
    renderer(renderer),
    texture_width(texture_width),
    texture_height(texture_height),
    texture(nullptr),
    pixel_format(nullptr),
    pixels(nullptr),
    pitch(0)
{
    std::uint32_t pixel_format_id = findPixelFormat(renderer);
    texture = SDL_CreateTexture(renderer, pixel_format_id,
                                SDL_TEXTUREACCESS_STREAMING, texture_width,
                                texture_height);
    if (texture == nullptr) {
        throw SdlException("Error creating a texture.", SDL_GetError());
    }
}

std::uint32_t SdlTexture::findPixelFormat(SDL_Renderer* renderer) {
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer, &info) != 0) {
        throw SdlException("Error getting renderer info.", SDL_GetError());
    }

    // Find a supported pixel format with 32 bits per pixel.
    std::uint_fast32_t pixel_format_id = SDL_PIXELFORMAT_UNKNOWN;
    for (unsigned int i = 0; i < 16 && i < info.num_texture_formats; i++) {
        std::uint_fast32_t candidate = info.texture_formats[i];
        if (SDL_BITSPERPIXEL(candidate) == 32) {
            pixel_format_id = candidate;
        }
    }
    if (pixel_format_id == SDL_PIXELFORMAT_UNKNOWN) {
        throw SdlException("No useful pixel format supported.", SDL_GetError());
    }
    pixel_format = SDL_AllocFormat(pixel_format_id);
    if (pixel_format == nullptr) {
        throw SdlException("Error creating the pixel format.", SDL_GetError());
    }
    return pixel_format_id;
}

SdlTexture::~SdlTexture() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
    }
    if (pixel_format != nullptr) {
        SDL_FreeFormat(pixel_format);
    }
}

void SdlTexture::lockForWriting() {
    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0) {
        throw SdlException("Error locking texture.", SDL_GetError());
    }
}

void SdlTexture::unlock() {
    SDL_UnlockTexture(texture);
    pixels = nullptr;
    pitch = 0;
}

bool SdlTexture::isLocked() const {
    return pixels != nullptr;
}

void SdlTexture::setPixel(unsigned int x, unsigned int y, const Color& color) {
    if (x >= texture_width || y >= texture_height) {
        // Coordinates out of bounds, return gracefully.
        return;
    }
    unsigned int index = texture_width * y + x;
    setPixelIndex(index, color);
}

void SdlTexture::setPixelIndex(unsigned int index, const Color& color) {
    assert(isLocked() && "Attempt to set pixels on unlocked texture.");
    assert(index < texture_width * texture_height && "Pixel index out of "
           "range.");
    std::uint32_t pixel_value = SDL_MapRGB(pixel_format, color[0], color[1],
                                           color[2]);
    std::uint32_t* pixels32 = static_cast<std::uint32_t*>(pixels);
    pixels32[index] = pixel_value;
}

void SdlTexture::setPixelRange(unsigned int start, unsigned int end,
                               const Color& color) {
    assert(isLocked() && "Attempt to set pixels on unlocked texture.");
    assert(start <= texture_width * texture_height && "Start index out of "
           "range.");
    assert(end <= texture_width * texture_height && "End index out of range.");
    if (start > end) {
        return;
    }
    std::uint32_t pixel_value = SDL_MapRGB(pixel_format, color[0], color[1],
                                           color[2]);
    std::uint32_t* pixels32 = static_cast<std::uint32_t*>(pixels);
    std::fill_n(pixels32 + start, end - start, pixel_value);
}

unsigned int SdlTexture::getWidth() const {
    return texture_width;
}

unsigned int SdlTexture::getHeight() const {
    return texture_height;
}

SDL_Texture* SdlTexture::getTexture() const {
    return texture;
}

}
