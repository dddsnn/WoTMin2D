#include "State.hpp"

namespace wotmin2d {

State::State(unsigned int width, unsigned int height) :
    width(width),
    height(height),
    blobs() {
    // TODO some blobs for testing     
    blobs.emplace_back(Vector(200, 200), 130, width, height);
    blobs.emplace_back(Vector(400, 400), 130, width, height);
    blobs.emplace_back(Vector(800, 600), 130, width, height);
}

void State::update() {

}

unsigned int State::getWidth() const {
    return width;
}

unsigned int State::getHeight() const {
    return height;
}

const std::vector<Blob>& State::getBlobs() const {
    return blobs;
}

}
