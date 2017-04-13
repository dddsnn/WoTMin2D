#ifndef BLOBSTATE_HPP
#define BLOBSTATE_HPP

#include "Direction.hpp"
#include "Particle.hpp"

#include <vector>
#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <array>
#include <chrono>

namespace wotmin2d {

template<class P = Particle>
class BlobState {
    private:
    using ParticleMap = std::unordered_map<IntVector, P*, IntVector::Hash>;
    class ParticleMobilityLess {
        public:
        bool operator()(const P* first, const P* second) const;
    };
    public:
    BlobState();
    BlobState(const BlobState&) = delete;
    BlobState& operator=(const BlobState&) = delete;
    ~BlobState();
    const std::vector<P*>& getParticles() const;
    const std::vector<P*> getParticles(const IntVector& center,
                                       float radius) const;
    void addParticle(const IntVector& position);
    void moveParticle(P& particle, Direction movement_direction);
    void collideParticles(P& first, P& second, Direction collision_direction);
    void advanceParticles(std::chrono::milliseconds time_delta);
    P* getHighestMobilityParticle();
    void addParticleFollowers(P& leader, const std::vector<P*>& followers);
    private:
    std::vector<P*> particles;
    ParticleMap particle_map;
    void updateParticleInformation(P& particle,
                                   const IntVector& old_position);
    void updateParticleMap(P& particle, const IntVector& old_position);
    void updateParticleNeighbors(P& particle);
};

}

#include "BlobState.tpp"

#endif
