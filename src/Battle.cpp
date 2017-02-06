#include "Battle.hpp"

namespace wotmin2d {

Battle::Battle(unsigned int arena_width, unsigned int arena_height) :
    screen(arena_width, arena_height),
    state(arena_width, arena_height) {
}

void Battle::start() {
    // TODO implement stopping
    std::uint_fast32_t time = SDL_GetTicks();
    // Render 200 frames at 100 fps.
    for (int i = 0; i < 200; i++) {
        state.update();
        screen.draw(state);
        // TODO unhardcode time step
        std::uint_fast32_t new_time = SDL_GetTicks();
        if (new_time < time + 10) {
            // Wait some time so that the new frame begins after the right time.
            SDL_Delay(time + 10 - new_time);
        }
        time = new_time;
    }
}

void Battle::stop() {

}

}
