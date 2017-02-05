#ifndef BLOB_H
#define BLOB_H

#include "Particle.hpp"

#include <vector>
#include <memory>

namespace wotmin2d {

class Blob {
    public:
    Blob();
    const std::vector<std::shared_ptr<Particle>>& getParticles() const;
    private:
    std::vector<std::shared_ptr<Particle>> particles;
};

}

#endif
