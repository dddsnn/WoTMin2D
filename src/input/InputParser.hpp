#ifndef INPUTPARSER_HPP
#define INPUTPARSER_HPP

#include "InputAction.hpp"
#include "../game/State.hpp"

#include <SDL.h>
#include <memory>
#include <cstdint>
#include <iterator>

namespace wotmin2d {

class InputParser {
    public:
    InputParser();
    ~InputParser();
    InputParser(const InputParser&) = delete;
    InputParser& operator=(const InputParser&) = delete;
    // Needs to be called from the main loop.
    std::vector<std::unique_ptr<InputAction>> parseInput();
    private:
    static int isMouseOrKeyDownEvent(void*, SDL_Event* event);
    static std::unique_ptr<InputAction> parseMouseDown(SDL_Event& event);
    static std::unique_ptr<InputAction> parseKeyDown(SDL_Event& event);
    template<class P, class I>
    void addActions(std::uint32_t type, P parser, I inserter);
    int getEvents(std::uint32_t type);
    constexpr static int batch_size = 10;
    SDL_Event* event_buffer;
};

template<class P, class I>
void InputParser::addActions(std::uint32_t type, P parser, I inserter) {
    int num_events = getEvents(type);
    while (num_events > 0) {
        assert(num_events >= 0);
        unsigned int unsigned_num_events = num_events;
        for (unsigned int i = 0; i < unsigned_num_events; i++) {
            std::unique_ptr<InputAction> action = parser(event_buffer[i]);
            if (action != nullptr) {
                inserter = parser(event_buffer[i]);
            }
        }
        num_events = getEvents(type);
    }

}

}

#endif
