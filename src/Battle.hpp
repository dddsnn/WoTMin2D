#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "game/State.hpp"
#include "display/Screen.hpp"

#include <SDL_timer.h>
#include <cstdint>

namespace wotmin2d {

class Battle {
    public:
    Battle(unsigned int width, unsigned int height);
    void start();
    void stop();

    private:
    Screen screen;
    State state;
};

}

#endif
