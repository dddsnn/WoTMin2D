#ifndef STATE_HPP
#define STATE_HPP

#include "Blob.hpp"
#include "Vector.hpp"
#include "../Config.hpp"

#include <vector>
#include <unordered_map>
#include <queue>
#include <tuple>
#include <chrono>
#include <iterator>
#include <algorithm>
#include <limits>
#include <utility>
#include <cstdint>

namespace wotmin2d {

template<class P = Particle, class B = Blob<P>>
class State {
    public:
    using PlayerId = std::uint_fast8_t;
    class BlobMobilityLess {
        public:
        bool operator()(const std::pair<PlayerId, B*>& first,
                        const std::pair<PlayerId, B*>& second) const;
    };
    State(unsigned int arena_width, unsigned int arena_height);
    void advance(std::chrono::milliseconds time_delta);
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    float getSelectionRadius() const;
    template<class... Args>
    void emplaceBlob(PlayerId player_id, Args... args);
    void changeSelectionRadius(float difference);
    const std::unordered_map<PlayerId, B>& getBlobs() const;
    void selectParticles(const IntVector& center);
    void setTarget(PlayerId player, const IntVector& target);
    private:
    const unsigned int arena_width;
    const unsigned int arena_height;
    std::unordered_map<PlayerId, B> blobs;
    IntVector selection_center;
    float selection_radius;
    bool isMovementOutOfBounds(const IntVector& position,
                               Direction movement_direction) const;
    bool isHostileCollision(const P& particle, Direction movement_direction,
                            PlayerId player_id);
    void handleParticle(P& particle, B& blob, PlayerId player_id);
};

}

#include "State.tpp"

#endif
