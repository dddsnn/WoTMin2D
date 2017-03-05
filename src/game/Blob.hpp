#ifndef BLOB_HPP
#define BLOB_HPP

#include "Particle.hpp"
#include "Vector.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <cassert>
#include <utility>
#include <array>

namespace wotmin2d {

class Blob {
    using ParticleMap = std::unordered_map<IntVector, std::shared_ptr<Particle>,
                                           IntVector::Hash>;
    public:
    Blob(const unsigned int& arena_width, const unsigned int& arena_height);
    Blob(const IntVector& center, float radius, const unsigned int& width,
         const unsigned int& arena_height);
    const std::vector<std::shared_ptr<Particle>>& getParticles() const;
    void advance();
    private:
    std::vector<std::shared_ptr<Particle>> particles;
    ParticleMap particle_map;
    const unsigned int& arena_width;
    const unsigned int& arena_height;
    void addParticleConnected(const IntVector& position);
    void advanceParticle(const std::shared_ptr<Particle>& particle);
    void updateParticleInformation(const std::shared_ptr<Particle>& particle,
                                     const IntVector& old_position);
    void updateParticleMap(const std::shared_ptr<Particle>& particle,
                             const IntVector& old_position);
    void updateParticleNeighbors(const std::shared_ptr<Particle>& particle);
    void moveParticleLine(std::shared_ptr<Particle> first_particle,
                          Direction movement_direction);
    bool dragParticlesBehindLine(std::shared_ptr<Particle> particle,
                                 Direction forward_direction,
                                 Direction line_direction);
};

}

#endif
