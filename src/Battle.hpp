#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "game/State.hpp"
#include "display/Screen.hpp"

#include <cstdint>
#include <chrono>
#include <thread>

namespace wotmin2d {

class Battle {
    public:
    Battle(unsigned int arena_width, unsigned int arena_height,
           unsigned int display_width, unsigned int display_height);
    void start();
    void stop();
    const static std::chrono::milliseconds frame_time;
    private:
    Screen screen;
    State state;
};

}

#endif
