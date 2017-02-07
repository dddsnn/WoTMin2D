#ifndef BLOB_HPP
#define BLOB_HPP

#include "Particle.hpp"
#include "Vector.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_map>

namespace wotmin2d {

class Blob {
    using ParticleMap = std::unordered_map<IntVector, std::shared_ptr<Particle>,
                                           IntVector::Hash>;
    public:
    Blob(const unsigned int& arena_width, const unsigned int& arena_height);
    Blob(const IntVector& center, float radius, const unsigned int& width,
         const unsigned int& arena_height);
    const std::vector<std::shared_ptr<Particle>>& getParticles() const;
    void update();
    private:
    std::vector<std::shared_ptr<Particle>> particles;
    ParticleMap particle_map;
    const unsigned int& arena_width;
    const unsigned int& arena_height;
    void addParticle(IntVector position);
};

}

#endif
