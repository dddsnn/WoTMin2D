#include "State.hpp"

namespace wotmin2d {

State::State(unsigned int arena_width, unsigned int arena_height) :
    arena_width(arena_width),
    arena_height(arena_height),
    blobs() {
    // TODO some blobs for testing     
    blobs.emplace_back(IntVector(20, 20), 10, arena_width, arena_height);
}

void State::advance(std::chrono::milliseconds time_delta) {
    for (auto& blob: blobs) {
        blob.advance(time_delta);
    }
}

unsigned int State::getWidth() const {
    return arena_width;
}

unsigned int State::getHeight() const {
    return arena_height;
}

const std::vector<Blob<>>& State::getBlobs() const {
    return blobs;
}

void State::setTarget(const IntVector& target) {
    for (Blob<> blob: blobs) {
        blob.setTarget(target);
    }
}

}
