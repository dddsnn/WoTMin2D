#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(IntVector position) :
    position(position),
    neighbor_north(),
    neighbor_south(),
    neighbor_west(),
    neighbor_east() {
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
    switch (direction) {
    case Direction::north:
        return neighbor_north;
        break;
    case Direction::south:
        return neighbor_south;
        break;
    case Direction::west:
        return neighbor_west;
        break;
    case Direction::east:
        return neighbor_east;
        break;
    default:
        // This can't happen unless an unknown value is added to Direction.
        throw std::invalid_argument("Unknown direction.");
    }
}

}
