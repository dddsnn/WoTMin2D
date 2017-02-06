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
#include <memory>
#include <cassert>

namespace wotmin2d {

class Screen {
    public:
    Screen(unsigned int display_width, unsigned int display_height);
    Screen(const Screen& other);
    Screen& operator=(Screen other);
    ~Screen();
    friend void swap(Screen& first, Screen& second) noexcept;
    void draw(const State& state);
    private:
    void construct(unsigned int display_width, unsigned int display_height);
    void updateTexture(const State& state);
    void presentTexture();
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::unique_ptr<SdlTexture> texture;
};

}

#endif
