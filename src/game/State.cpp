#include "State.hpp"

namespace wotmin2d {

State::State(unsigned int arena_width, unsigned int arena_height) :
    arena_width(arena_width),
    arena_height(arena_height),
    blobs() {
    static_assert(Config::num_players == 1, "Can only handle exactly 1 "
                  "player.");
    blobs.emplace_back(0, IntVector(20, 20), 10.0f, arena_width, arena_height);
    blobs.shrink_to_fit();
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

void State::setTarget(PlayerId player, const IntVector& target) {
    assert(player < Config::num_players);
    // TODO Unhardcode pressure, center and radius.
    blobs[player].setTarget(target, 20.0f, IntVector(25, 10), 8.0f);
}

}
