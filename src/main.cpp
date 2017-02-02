#include <iostream>
#include <SDL.h>

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_Quit();
    return 0;
}
