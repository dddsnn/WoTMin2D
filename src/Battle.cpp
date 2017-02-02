#include "Battle.hpp"

namespace wotmin2d {

void Battle::start() {
    // TODO implement stopping
    while (true) {
        state.update();
        screen.draw(state);
        // TODO unhardcode time step
        SDL_Delay(10);
    }
}

void Battle::stop() {

}

}
