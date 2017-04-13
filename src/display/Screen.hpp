#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "../game/State.hpp"
#include "../game/Blob.hpp"
#include "../game/Particle.hpp"
#include "SdlTexture.hpp"
#include "SdlException.hpp"

#include <SDL.h>
#include <utility>
#include <cstdint>
#include <cassert>

namespace wotmin2d {

class Screen {
    public:
    Screen(unsigned int arena_width, unsigned int arena_height,
           unsigned int display_width, unsigned int display_height);
    Screen(const Screen& other);
    Screen& operator=(Screen other);
    ~Screen();
    friend void swap(Screen& first, Screen& second) noexcept;
    void draw(const State& state);
    IntVector scaleWindowToArenaCoordinates(const IntVector& coordinate) const;
    unsigned int invertArenaY(unsigned int y) const;
    IntVector sdlToArenaCoordinates(const IntVector& coordinate) const;
    private:
    void construct(unsigned int arena_width, unsigned int arena_height,
                   unsigned int display_width, unsigned int display_height);
    void updateTexture(const State& state);
    void presentTexture();
    void putBlobs(const State& state);
    void putSelectionCircleAndAimPoint(const State& state);
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::unique_ptr<SdlTexture> texture;
};

}

#endif
