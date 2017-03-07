#include "Battle.hpp"

namespace wotmin2d {

const std::chrono::milliseconds Battle::frame_time
    = std::chrono::milliseconds(500);

Battle::Battle(unsigned int arena_width, unsigned int arena_height,
               unsigned int display_width, unsigned int display_height) :
    screen(arena_width, arena_height, display_width, display_height),
    state(arena_width, arena_height) {
}

void Battle::start() {
    // TODO Check whether high_resolution_clock is steady. If not, weird
    // behavior could happen if the system time is adjusted.
    using std::chrono::high_resolution_clock;
    using time_point = high_resolution_clock::time_point;
    // TODO implement stopping
    // Render 100 frames.
    for (int i = 0; i < 10; i++) {
        time_point start_time = high_resolution_clock::now();
        state.update();
        screen.draw(state);
        time_point end_time = high_resolution_clock::now();
        if (end_time < start_time + frame_time) {
            // Wait some time so that the new frame begins after the right time.
            std::chrono::nanoseconds sleep_time = (start_time + frame_time)
                                                  - end_time;
            std::this_thread::sleep_for(sleep_time);
        }
    }
}

void Battle::stop() {

}

}
