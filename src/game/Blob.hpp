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
    Blob(const unsigned int& widht, const unsigned int& height);
    Blob(const Vector& center, float radius, const unsigned int& widht,
         const unsigned int& height);
    const std::vector<std::shared_ptr<Particle>>& getParticles() const;
    private:
    std::vector<std::shared_ptr<Particle>> particles;
    const unsigned int& width;
    const unsigned int& height;
};

}

#endif
