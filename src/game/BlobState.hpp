#ifndef BLOBSTATE_HPP
#define BLOBSTATE_HPP

#include "Direction.hpp"
#include "Particle.hpp"

#include <vector>
#include <unordered_map>
#include <memory>
#include <cassert>

namespace wotmin2d {

class BlobState {
    private:
    using ParticlePtr = std::shared_ptr<Particle>;
    using ParticleMap = std::unordered_map<IntVector, ParticlePtr,
                                           IntVector::Hash>;
    public:
    BlobState();
    const std::vector<ParticlePtr>& getParticles() const;
    void addParticle(const IntVector& position);
    void moveParticle(const ParticlePtr& particle,
                      Direction movement_direction);
    private:
    std::vector<ParticlePtr> particles;
    ParticleMap particle_map;
    void updateParticleInformation(const ParticlePtr& particle,
                                   const IntVector& old_position);
    void updateParticleMap(const ParticlePtr& particle,
                           const IntVector& old_position);
    void updateParticleNeighbors(const ParticlePtr& particle);
};

}

#endif
