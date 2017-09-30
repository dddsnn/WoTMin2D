#include "Screen.hpp"

namespace wotmin2d {

void Screen::construct(unsigned int arena_width,
                       unsigned int arena_height,
                       unsigned int display_width,
                       unsigned int display_height) {
    window = SDL_CreateWindow("WoTMin2D", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, display_width,
                              display_height, 0);
    if (window == nullptr) {
        throw SdlException("Error creating a window.", SDL_GetError());
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        throw SdlException("Error creating a renderer.", SDL_GetError());
    }
    try {
        texture.reset(new SdlTexture(renderer, arena_width, arena_height));
    } catch (...) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        throw;
    }
}

Screen::Screen(unsigned int arena_width, unsigned int arena_height,
               unsigned int display_width, unsigned int display_height) :
    window(nullptr),
    renderer(nullptr),
    texture(nullptr)
{
    construct(arena_width, arena_height, display_width, display_height);
}

Screen::Screen(const Screen& other) :
    window(nullptr),
    renderer(nullptr),
    texture(nullptr)
{
    int display_width, display_height;
    SDL_GetWindowSize(other.window, &display_width, &display_height);
    construct(other.texture->getWidth(), other.texture->getHeight(),
              display_width, display_height);
}

Screen& Screen::operator=(Screen other) {
    swap(*this, other);
    return *this;
}

Screen::~Screen() {
    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
    }
    if (window != nullptr) {
        SDL_DestroyWindow(window);
    }
}

void swap(Screen& first, Screen& second) noexcept {
    using std::swap;
    swap(first.window, second.window);
    swap(first.renderer, second.renderer);
    swap(first.texture, second.texture);
}

void Screen::draw(const State<>& state) {
    updateTexture(state);
    presentTexture();
}

IntVector Screen::scaleWindowToArenaCoordinates(const IntVector& coordinate)
    const
{
    int window_width_int, window_height_int;
    SDL_GetWindowSize(window, &window_width_int, &window_height_int);
    assert(coordinate.getX() >= 0);
    assert(coordinate.getY() >= 0);
    int x_in = coordinate.getX();
    int y_in = coordinate.getY();
    if (x_in >= window_width_int) {
        // This can happen even with valid inputs on window resizing. Set to
        // window border.
        x_in = window_width_int - 1;
    }
    if (y_in >= window_height_int) {
        y_in = window_height_int - 1;
    }
    float window_width = static_cast<float>(window_width_int);
    float window_height = static_cast<float>(window_height_int);
    float arena_width = static_cast<float>(texture->getWidth());
    float arena_height = static_cast<float>(texture->getHeight());
    float width_multiplier = arena_width / window_width;
    float height_multiplier = arena_height / window_height;
    float window_x = static_cast<float>(x_in);
    int arena_x = static_cast<int>(window_x * width_multiplier);
    float window_y = static_cast<float>(y_in);
    int arena_y = static_cast<int>(window_y * height_multiplier);
    return IntVector(arena_x, arena_y);
}

unsigned int Screen::invertArenaY(unsigned int y) const {
    return texture->getHeight() - y - 1;
}

IntVector Screen::sdlToArenaCoordinates(const IntVector& coordinate) const {
    IntVector arena_coordinate = scaleWindowToArenaCoordinates(coordinate);
    assert(arena_coordinate.getY() >= 0);
    unsigned int y_inv = invertArenaY(arena_coordinate.getY());
    return IntVector(arena_coordinate.getX(), y_inv);
}

void Screen::updateTexture(const State<>& state) {
    assert(!texture->isLocked() && "Attempt to update a texture that was "
           "already locked for writing.");
    texture->lockForWriting();
    // Make everything white to begin with.
    texture->setPixelRange(0, texture->getWidth() * texture->getHeight(), 0xff,
                           0xff, 0xff);
    putBlobs(state);
    // TODO Getting the mouse state here means it may be a bit old (since
    // rendering the blobs takes time).
    putSelectionCircleAndAimPoint(state);
    texture->unlock();
}

void Screen::putBlobs(const State<>& state) {
    assert(texture->isLocked() && "Attempt to set pixels on a texture that "
           "isn't locked for writing.");
    for (const Blob<>& blob: state.getBlobs()) {
        for (const Particle* particle: blob.getParticles()) {
            int x_signed = particle->getPosition().getX();
            int y_signed = particle->getPosition().getY();
            assert(x_signed >= 0 && "X coordinate must be positive");
            assert(y_signed >= 0 && "Y coordinate must be positive");
            unsigned int x = static_cast<unsigned int>(x_signed);
            unsigned int y = static_cast<unsigned int>(y_signed);
            assert(x < state.getWidth() && "X coordinate must be less than "
                                           "arena width.");
            assert(y < state.getHeight() && "Y coordinate must be less than "
                                            "arena height.");
            // The y-coordinates of particle start at the bottom, increasing
            // towards the top, texture coordinates are the other way around.
            unsigned int y_arena = invertArenaY(y);
            texture->setPixel(x, y_arena, 0xff, 0x00, 0x00);
        }
    }
}

void Screen::putSelectionCircleAndAimPoint(const State<>& state) {
    assert(texture->isLocked() && "Attempt to set pixels on a texture that "
           "isn't locked for writing.");
    // TODO A bit ugly that I'm getting the mouse state here.
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    IntVector mouse_position
        = scaleWindowToArenaCoordinates(IntVector(mouse_x, mouse_y));
    float radius = state.getSelectionRadius();
    assert(radius >= 0.0f);
    int radius_int = static_cast<int>(radius);
    int squared_radius = static_cast<int>(radius * radius);
    int x = 0;
    int y = radius_int;
    assert(IntVector(x, y).squaredNorm() <= squared_radius);
    while (y >= 0) {
        assert(IntVector(x, y).squaredNorm() <= squared_radius);
        texture->setPixel(mouse_position.getX() + x, mouse_position.getY() + y,
                          0x00, 0x00, 0x00);
        texture->setPixel(mouse_position.getX() + x, mouse_position.getY() - y,
                          0x00, 0x00, 0x00);
        texture->setPixel(mouse_position.getX() - x, mouse_position.getY() + y,
                          0x00, 0x00, 0x00);
        texture->setPixel(mouse_position.getX() - x, mouse_position.getY() - y,
                          0x00, 0x00, 0x00);
        if (IntVector(x + 1, y).squaredNorm() <= squared_radius) {
            x++;
        } else {
            y--;
        }
    }
    // Aim point
    texture->setPixel(mouse_position.getX(), mouse_position.getY(), 0x00, 0x00,
                      0x00);
}

void Screen::presentTexture() {
    assert(!texture->isLocked() && "Attempt to present a texture that's "
           "currently locked for writing.");
    if (SDL_RenderClear(renderer) != 0) {
        throw SdlException("Error clearing the renderer.", SDL_GetError());
    }
    if (SDL_RenderCopy(renderer, texture->getTexture(), nullptr,
                       nullptr) != 0) {
        throw SdlException("Error copying a texture to the renderer.",
                           SDL_GetError());
    }
    SDL_RenderPresent(renderer);
}

}
