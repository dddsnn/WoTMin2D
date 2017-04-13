#include "State.hpp"

namespace wotmin2d {

State::State(unsigned int arena_width, unsigned int arena_height) :
    arena_width(arena_width),
    arena_height(arena_height),
    blobs(),
    selected_center(),
    selected_radius(0.0f) {
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

void State::selectParticles(PlayerId player, const IntVector& center,
                            float radius) {
    assert(player == 0 && "Can only select particless for player 0.");
    selected_center = center;
    selected_radius = radius;
}

void State::setTarget(PlayerId player, const IntVector& target) {
    assert(player < Config::num_players);
    // TODO Unhardcode pressure.
    blobs[player].setTarget(target, 20.0f, selected_center, selected_radius);
}

}
