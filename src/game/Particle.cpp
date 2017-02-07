#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(Vector position) :
    position_(position),
    neighbor_north(),
    neighbor_south(),
    neighbor_west(),
    neighbor_east() {
}

const Vector& Particle::getPosition() const {
    return position_;
}

void Particle::move(Direction direction) {
    switch (direction) {
    case Direction::north:
        position_.y()--;
        break;
    case Direction::south:
        position_.y()++;
        break;
    case Direction::west:
        position_.x()--;
        break;
    case Direction::east:
        position_.x()++;
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
