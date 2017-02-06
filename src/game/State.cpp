#include "State.hpp"

namespace wotmin2d {

State::State(unsigned int arena_width, unsigned int arena_height) :
    arena_width(arena_width),
    arena_height(arena_height),
    blobs() {
    // TODO some blobs for testing     
    blobs.emplace_back(Vector(200, 200), 130, arena_width, arena_height);
    blobs.emplace_back(Vector(400, 400), 130, arena_width, arena_height);
    blobs.emplace_back(Vector(800, 600), 130, arena_width, arena_height);
}

void State::update() {

}

unsigned int State::getWidth() const {
    return arena_width;
}

unsigned int State::getHeight() const {
    return arena_height;
}

const std::vector<Blob>& State::getBlobs() const {
    return blobs;
}

}
