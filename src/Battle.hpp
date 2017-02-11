#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "game/State.hpp"
#include "display/Screen.hpp"

#include <SDL_timer.h>
#include <cstdint>

namespace wotmin2d {

class Battle {
    public:
    Battle(unsigned int arena_width, unsigned int arena_height,
           unsigned int display_width, unsigned int display_height);
    void start();
    void stop();
    constexpr static std::uint_fast32_t frame_ticks = 50;
    private:
    Screen screen;
    State state;
};

}

#endif
