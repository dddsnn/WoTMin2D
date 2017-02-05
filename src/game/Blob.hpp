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
    Blob();
    Blob(const Vector& center, float radius, const Vector& bottom_right);
    const std::vector<std::shared_ptr<Particle>>& getParticles() const;
    private:
    std::vector<std::shared_ptr<Particle>> particles;
};

}

#endif
