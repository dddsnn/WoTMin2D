#ifndef BLOB_HPP
#define BLOB_HPP

#include "BlobState.hpp"
#include "Particle.hpp"
#include "Vector.hpp"
#include "../Config.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <cassert>
#include <utility>
#include <array>
#include <unordered_set>
#include <chrono>
#include <cstdint>

namespace wotmin2d {

template<class P = Particle, class B = BlobState<P>>
class Blob {
    public:
    using PlayerId = std::uint_fast8_t;
    Blob(PlayerId player_id, unsigned int arena_width,
         unsigned int arena_height,
         std::shared_ptr<B> state = std::make_shared<B>());
    Blob(PlayerId player_id, const IntVector& center, float radius,
         unsigned int arena_width, unsigned int arena_height,
         std::shared_ptr<B> state = std::make_shared<B>());
    const std::vector<P*>& getParticles() const;
    void advance(std::chrono::milliseconds time_delta);
    void setTarget(const IntVector& target, float pressure_per_second,
                   const IntVector& center, float radius);
    private:
    bool isMovementOutOfBounds(const IntVector& position,
                               Direction movement_direction) const;
    const PlayerId player_id;
    // TODO Store by value and make the tests a friend so they can replace it.
    std::shared_ptr<B> state;
    unsigned int arena_width;
    unsigned int arena_height;
    void handleParticle(P& particle);
};

}

#include "Blob.tpp"

#endif
