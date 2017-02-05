#ifndef BLOB_H
#define BLOB_H

#include "Particle.hpp"

#include <vector>
#include <memory>
#include <algorithm>

namespace wotmin2d {

class Blob {
    public:
    Blob();
    Blob(unsigned int center_x, unsigned int center_y, float radius,
         unsigned int max_x, unsigned int max_y);
    const std::vector<std::shared_ptr<Particle>>& getParticles() const;
    private:
    std::vector<std::shared_ptr<Particle>> particles;
};

}

#endif
