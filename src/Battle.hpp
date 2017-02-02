#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "game/State.hpp"
#include "display/Screen.hpp"

#include <SDL_timer.h>

namespace wotmin2d {

class Battle {
    public:
    void start();
    void stop();

    private:
    Screen screen;
    State state;
};

}

#endif
