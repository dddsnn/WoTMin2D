#ifndef BLOBSTATE_HPP
#define BLOBSTATE_HPP

#include "Direction.hpp"
#include "Particle.hpp"

#include <vector>
#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <array>

namespace wotmin2d {

class BlobState {
    private:
    using ParticleMap = std::unordered_map<IntVector, Particle*,
                                           IntVector::Hash>;
    class ParticleMobilityLess {
        public:
        bool operator()(const Particle* first,
                        const Particle* second) const;
    };
    public:
    BlobState();
    ~BlobState();
    const std::vector<Particle*>& getParticles() const;
    void addParticle(const IntVector& position);
    void moveParticle(Particle& particle, Direction movement_direction);
    void collideParticles(Particle& first, Particle& second,
                          Direction collision_direction);
    void advanceParticles();
    Particle* getHighestMobilityParticle();
    void addParticleFollowers(Particle& leader,
                              const std::vector<Particle*>& followers);
    private:
    std::vector<Particle*> particles;
    ParticleMap particle_map;
    void updateParticleInformation(Particle& particle,
                                   const IntVector& old_position);
    void updateParticleMap(Particle& particle, const IntVector& old_position);
    void updateParticleNeighbors(Particle& particle);
};

}

#endif
