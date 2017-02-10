#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(IntVector position) :
    position(position),
    neighbors() {
}

const IntVector& Particle::getPosition() const {
    return position;
}

void Particle::move(Direction direction) {
    position += direction.vector();
}

std::shared_ptr<Particle>& Particle::neighbor(Direction direction) {
    // Direction is convertible to unsigned integers, starting at 0.
    return neighbors[static_cast<Direction::val_t>(direction)];
}

}
