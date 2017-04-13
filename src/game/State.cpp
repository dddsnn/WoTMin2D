#include "State.hpp"

namespace wotmin2d {

State::State(unsigned int arena_width, unsigned int arena_height) :
    arena_width(arena_width),
    arena_height(arena_height),
    blobs(),
    selection_center(),
    selection_radius(5.0f) {
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

float State::getSelectionRadius() const {
    return selection_radius;
}

void State::changeSelectionRadius(float difference) {
    selection_radius += difference;
    if (selection_radius < Config::min_selection_radius) {
        selection_radius = Config::min_selection_radius;
    }
}

const std::vector<Blob<>>& State::getBlobs() const {
    return blobs;
}

void State::selectParticles(PlayerId player, const IntVector& center) {
    assert(player == 0 && "Can only select particless for player 0.");
    selection_center = center;
}

void State::setTarget(PlayerId player, const IntVector& target) {
    assert(player < Config::num_players);
    // TODO Unhardcode pressure.
    blobs[player].setTarget(target, 20.0f, selection_center, selection_radius);
}

}
