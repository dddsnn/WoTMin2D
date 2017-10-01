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
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>

namespace wotmin2d {

namespace mi = boost::multi_index;

template<class P = Particle>
class BlobState {
    public:
    class ParticleMobilityGreater {
        public:
        bool operator()(const P* first, const P* second) const;
    };
    using ParticleMap = std::unordered_map<IntVector, P*, IntVector::Hash>;
    using ParticleSet = mi::multi_index_container<
        P*,
        mi::indexed_by<
            mi::hashed_unique<mi::identity<P*>>,
            mi::ordered_non_unique<mi::identity<P*>, ParticleMobilityGreater>
        >
    >;
    BlobState();
    BlobState(const BlobState&) = delete;
    BlobState& operator=(const BlobState&) = delete;
    ~BlobState();
    const ParticleSet& getParticles() const;
    const std::vector<P*> getParticles(const IntVector& center,
                                       float radius) const;
    const P* getParticleAt(const IntVector& position) const;
    void addParticle(const IntVector& position);
    void moveParticle(P& particle, Direction movement_direction);
    void collideParticles(P& first, P& second, Direction collision_direction);
    void collideParticleWithWall(P& particle, Direction collision_direction);
    void advanceParticles(std::chrono::milliseconds time_delta);
    P* getHighestMobilityParticle();
    void addParticleFollowers(P& leader, const std::vector<P*>& followers);
    private:
    ParticleSet particles;
    ParticleMap particle_map;
    void updateParticleInformation(P& particle,
                                   const IntVector& old_position);
    void updateParticleMap(P& particle, const IntVector& old_position);
    void updateParticleNeighbors(P& particle);
    template<class Modifier>
    void modifyParticle(P& particle, Modifier modifier);
};

}

#include "BlobState.tpp"

#endif
