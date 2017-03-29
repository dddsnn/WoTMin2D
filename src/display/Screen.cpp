#include "Screen.hpp"

namespace wotmin2d {

void Screen::construct(unsigned int arena_width,
                       unsigned int arena_height,
                       unsigned int display_width,
                       unsigned int display_height) {
    window = SDL_CreateWindow("WoTMin2D", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, display_width,
                              display_height, 0);
    if (window == nullptr) {
        throw SdlException("Error creating a window.", SDL_GetError());
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        throw SdlException("Error creating a renderer.", SDL_GetError());
    }
    try {
        texture.reset(new SdlTexture(renderer, arena_width, arena_height));
    } catch (...) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        throw;
    }
}

Screen::Screen(unsigned int arena_width, unsigned int arena_height,
               unsigned int display_width, unsigned int display_height) :
    window(nullptr),
    renderer(nullptr),
    texture(nullptr)
{
    construct(arena_width, arena_height, display_width, display_height);
}

Screen::Screen(const Screen& other) :
    window(nullptr),
    renderer(nullptr),
    texture(nullptr)
{
    int display_width, display_height;
    SDL_GetWindowSize(other.window, &display_width, &display_height);
    construct(other.texture->getWidth(), other.texture->getHeight(),
              display_width, display_height);
}

Screen& Screen::operator=(Screen other) {
    swap(*this, other);
    return *this;
}

Screen::~Screen() {
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
    }
    if (window != nullptr) {
        SDL_DestroyWindow(window);
    }
}

void swap(Screen& first, Screen& second) noexcept {
    using std::swap;
    swap(first.window, second.window);
    swap(first.renderer, second.renderer);
    swap(first.texture, second.texture);
}

void Screen::draw(const State& state) {
    updateTexture(state);
    presentTexture();
}

void Screen::updateTexture(const State& state) {
    texture->lockForWriting();
    // Make everything white to begin with.
    texture->setRange(0, texture->getWidth() * texture->getHeight(), 0xff, 0xff,
                      0xff);
    for (const Blob<>& blob: state.getBlobs()) {
        for (const auto& particle: blob.getParticles()) {
            int x_signed = particle->getPosition().getX();
            int y_signed = particle->getPosition().getY();
            assert(x_signed >= 0 && "X coordinate must be positive");
            assert(y_signed >= 0 && "Y coordinate must be positive");
            unsigned int x = static_cast<unsigned int>(x_signed);
            unsigned int y = static_cast<unsigned int>(y_signed);
            assert(x < state.getWidth() && "X coordinate must be less than "
                                           "arena width.");
            assert(y < state.getHeight() && "Y coordinate must be less than "
                                            "arena height.");
            // The y-coordinates of particle start at the bottom, increasing
            // towards the top, texture coordinates are the other way around.
            unsigned int y_top_bottom = texture->getHeight() - y - 1;
            unsigned int index = texture->getWidth() * y_top_bottom + x;
            texture->setPixel(index, 0xff, 0x00, 0x00);
        }
    }
    texture->unlock();
}

void Screen::presentTexture() {
    if (SDL_RenderClear(renderer) != 0) {
        throw SdlException("Error clearing the renderer.", SDL_GetError());
    }
    if (SDL_RenderCopy(renderer, texture->getTexture(), nullptr,
                       nullptr) != 0) {
        throw SdlException("Error copying a texture to the renderer.",
                           SDL_GetError());
    }
    SDL_RenderPresent(renderer);
}

}
