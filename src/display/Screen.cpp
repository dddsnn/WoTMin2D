#include "Screen.hpp"

namespace wotmin2d {

void Screen::construct(unsigned int width, unsigned int height) {
    window = SDL_CreateWindow("WoTMin2D", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, width, height, 0);
    if (window == nullptr) {
        throw SdlException("Error creating a window.", SDL_GetError());
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        throw SdlException("Error creating a renderer.", SDL_GetError());
    }
    try {
        texture.reset(new SdlTexture(renderer, width, height));
    } catch (...) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        throw;
    }
}

Screen::Screen(unsigned int width, unsigned int height) :
    window(nullptr),
    renderer(nullptr),
    texture(nullptr)
{
    construct(width, height);
}

Screen::Screen(const Screen& other) :
    window(nullptr),
    renderer(nullptr),
    texture(nullptr)
{
    int width, height;
    SDL_GetWindowSize(other.window, &width, &height);
    construct(width, height);
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
    for (const Blob& blob: state.getBlobs()) {
        for (const std::shared_ptr<Particle>& particle: blob.getParticles()) {
            unsigned int index = texture->getWidth() * particle->getY() +
                                 particle->getX();
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
