#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(IntVector position) :
    position(position),
    target(0, 0),
    target_pressure(0.0f),
    pressure(1.0f, 0.0f), // TODO this is for testing     
    // TODO pressure(0.0f, 0.0f),
    neighbors() {
}

const IntVector& Particle::getPosition() const {
    return position;
}

std::shared_ptr<Particle>& Particle::neighbor(Direction direction) {
    // Direction is convertible to unsigned integers, starting at 0.
    return neighbors[static_cast<Direction::val_t>(direction)];
}

Particle::Movement Particle::getMovement() const {
    Direction pressure_direction = getPressureDirection();
    return std::make_pair(pressure_direction,
                          pressure != FloatVector(0.0f, 0.0f));
}

void Particle::move(Direction direction) {
    position += direction.vector();
}

void Particle::collideWith(Particle& forward_neighbor) {
    forward_neighbor.pressure += pressure;
    pressure = FloatVector(0.0f, 0.0f);
}

Direction Particle::getPressureDirection() const {
    const float x = pressure.getX();
    const float y = pressure.getY();
    const float x_abs = std::abs(x);
    const float y_abs = std::abs(y);
    if (x_abs > y_abs) {
        if (x > 0.0f) {
            return Direction::east();
        } else {
            return Direction::west();
        }
    } else {
        if (y > 0.0f) {
            return Direction::north();
        } else {
            return Direction::south();
        }
    }
}

}
