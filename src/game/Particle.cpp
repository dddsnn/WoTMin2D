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
    switch (direction) {
    case Direction::north:
        position.setY(position.getY() - 1);
        break;
    case Direction::south:
        position.setY(position.getY() + 1);
        break;
    case Direction::west:
        position.setX(position.getX() - 1);
        break;
    case Direction::east:
        position.setX(position.getX() + 1);
        break;
    }
}

std::shared_ptr<Particle>& Particle::neighbor(Direction direction) {
    // Direction is of underlying type unsigned int and has values 0, 1, 2 and
    // 3.
    return neighbors[static_cast<unsigned int>(direction)];
}

}
