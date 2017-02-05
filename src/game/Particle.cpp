#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(Vector position) :
    position_(position) {
}

const Vector& Particle::getPosition() const {
    return position_;
}

}
