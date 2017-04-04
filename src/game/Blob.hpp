#ifndef BLOB_HPP
#define BLOB_HPP

#include "BlobState.hpp"
#include "Particle.hpp"
#include "Vector.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <cassert>
#include <utility>
#include <array>
#include <unordered_set>
#include <chrono>

namespace wotmin2d {

template<class BlobState = BlobState, class Particle = Particle>
class Blob {
    public:
    Blob(unsigned int arena_width, unsigned int arena_height,
         std::shared_ptr<BlobState> state = std::make_shared<BlobState>());
    Blob(const IntVector& center, float radius, unsigned int arena_width,
         unsigned int arena_height,
         std::shared_ptr<BlobState> state = std::make_shared<BlobState>());
    const std::vector<Particle*>& getParticles() const;
    void advance(std::chrono::milliseconds time_delta);
    private:
    // TODO Store by value and make the tests a friend so they can replace it.
    std::shared_ptr<BlobState> state;
    unsigned int arena_width;
    unsigned int arena_height;
    void handleParticle(Particle& particle);
};

}

#include "Blob.tpp"

#endif
