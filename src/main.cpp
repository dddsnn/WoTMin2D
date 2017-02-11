#include "Battle.hpp"

#include <iostream>
#include <SDL.h>

int main(int, char**) {
    {
        int code = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
        if (code != 0) {
            std::cerr << SDL_GetError() << std::endl;
            return code;
        }
    }

    wotmin2d::Battle b(100, 100, 1000, 1000);
    b.start();

    SDL_Quit();
    return 0;
}
