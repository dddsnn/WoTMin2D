#include "Blob.hpp"

namespace wotmin2d {

Blob::Blob() :
    particles() {
}

const std::vector<std::shared_ptr<Particle>>& Blob::getParticles() const {
    return particles;
}

}
