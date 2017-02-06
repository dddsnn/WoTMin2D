#ifndef BLOB_HPP
#define BLOB_HPP

#include "Particle.hpp"
#include "Vector.hpp"

#include <vector>
#include <memory>
#include <algorithm>

namespace wotmin2d {

class Blob {
    public:
    Blob(const unsigned int& arena_width, const unsigned int& arena_height);
    Blob(const Vector& center, float radius, const unsigned int& widht,
         const unsigned int& arena_height);
    const std::vector<std::shared_ptr<Particle>>& getParticles() const;
    void update();
    private:
    std::vector<std::shared_ptr<Particle>> particles;
    const unsigned int& arena_width;
    const unsigned int& arena_height;
};

}

#endif
