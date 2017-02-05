#include "State.hpp"

namespace wotmin2d {

State::State(unsigned int width, unsigned int height) :
    width(width),
    height(height),
    blobs() {
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
