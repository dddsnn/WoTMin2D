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

namespace wotmin2d {

template<class BlobState = BlobState, class Particle = Particle>
class Blob {
    private:
    using ParticlePtr = std::shared_ptr<Particle>;
    public:
    Blob(unsigned int arena_width, unsigned int arena_height,
         std::shared_ptr<BlobState> state = std::make_shared<BlobState>());
    Blob(const IntVector& center, float radius, unsigned int arena_width,
         unsigned int arena_height,
         std::shared_ptr<BlobState> state = std::make_shared<BlobState>());
    const std::vector<ParticlePtr>& getParticles() const;
    void advance();
    private:
    std::shared_ptr<BlobState> state;
    unsigned int arena_width;
    unsigned int arena_height;
    void handleParticle(const ParticlePtr& particle);
    void moveParticleLine(ParticlePtr first_particle,
                          Direction movement_direction);
    bool dragParticlesBehindLine(ParticlePtr particle,
                                 Direction forward_direction,
                                 Direction line_direction,
                                 ParticlePtr last_line_particle,
                                 unsigned int line_length);
};

}

#include "Blob.tpp"

#endif
