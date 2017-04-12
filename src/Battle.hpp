#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "game/State.hpp"
#include "display/Screen.hpp"
#include "input/InputParser.hpp"
#include "input/InputAction.hpp"

#include <cstdint>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>

namespace wotmin2d {

class Battle {
    public:
    Battle(unsigned int arena_width, unsigned int arena_height,
           unsigned int display_width, unsigned int display_height);
    void start();
    void stop();
    const static std::chrono::milliseconds frame_time;
    private:
    void handleInput(std::vector<std::unique_ptr<InputAction>>& actions);
    void handleMouseDown(const IntVector& coordinate);
    Screen screen;
    State state;
    InputParser input_parser;
    bool running;
};

}

#endif
