#ifndef STATE_HPP
#define STATE_HPP

#include "Blob.hpp"
#include "Vector.hpp"
#include "../Config.hpp"

#include <vector>
#include <queue>
#include <chrono>
#include <iterator>
#include <algorithm>
#include <limits>
#include <utility>

namespace wotmin2d {

template<class P = Particle, class B = Blob<P>>
class State {
    public:
    class BlobMobilityLess {
        public:
        bool operator()(const B* first, const B* second) const;
    };
    using PlayerId = typename B::PlayerId;
    State(unsigned int arena_width, unsigned int arena_height);
    void advance(std::chrono::milliseconds time_delta);
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    float getSelectionRadius() const;
    template<class... Args>
    void emplaceBlob(Args... args);
    void changeSelectionRadius(float difference);
    const std::vector<B>& getBlobs() const;
    void selectParticles(const IntVector& center);
    void setTarget(PlayerId player, const IntVector& target);
    private:
    const unsigned int arena_width;
    const unsigned int arena_height;
    std::vector<B> blobs;
    IntVector selection_center;
    float selection_radius;
    bool isMovementOutOfBounds(const IntVector& position,
                               Direction movement_direction) const;
    bool isHostileCollision(const P& particle, Direction movement_direction,
                            typename B::PlayerId player_id);
    void handleParticle(P& particle, B& blob);
};

}

#include "State.tpp"

#endif
