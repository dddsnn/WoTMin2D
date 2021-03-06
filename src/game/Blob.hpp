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

namespace wotmin2d {

template<class P = Particle, class B = BlobState<P>>
class Blob {
    public:
    Blob(std::shared_ptr<B> state = std::make_shared<B>());
    Blob(const IntVector& center, float radius,
         unsigned int arena_width, unsigned int arena_height,
         std::shared_ptr<B> state = std::make_shared<B>());
    void damageParticle(P& particle, int advantage);
    const typename BlobState<P>::ParticleSet& getParticles() const;
    P* getParticleAt(const IntVector& position) const;
    void advanceParticles(std::chrono::milliseconds time_delta);
    P* getHighestMobilityParticle() const;
    void setTarget(const IntVector& target, float pressure_per_second,
                   const IntVector& center, float radius);
    void collideParticleWithWall(P& particle, Direction collision_direction);
    void handleParticle(P& particle, Direction movement_direction);
    int getParticleStrength(const P& particle) const;
    private:
    // TODO Store by value and make the tests a friend so they can replace it.
    std::shared_ptr<B> state;
};

}

#include "Blob.tpp"

#endif
