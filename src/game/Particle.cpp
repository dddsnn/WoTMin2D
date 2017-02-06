#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(Vector position) :
    position_(position) {
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

}
