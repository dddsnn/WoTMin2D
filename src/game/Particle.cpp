#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(IntVector position) :
    position(position),
    target(0, 0),
    target_pressure(0.0f),
    pressure(0.0f, 0.0f),
    neighbors() {
    setTarget(IntVector(50, 30), 1.0f); // TODO here for testing
}

const IntVector& Particle::getPosition() const {
    return position;
}

const std::shared_ptr<Particle>& Particle::getNeighbor(Direction direction)
    const {
    // Direction is convertible to unsigned integers, starting at 0.
    return neighbors[static_cast<Direction::val_t>(direction)];
}

void Particle::setNeighbor(MoveKey, Direction direction,
                           const std::shared_ptr<Particle>& neighbor) {
    neighbors[static_cast<Direction::val_t>(direction)] = neighbor;
}

bool Particle::hasPath(std::initializer_list<Direction> directions) const {
    if (directions.size() == 0) {
        return true;
    }
    auto direction_iter = directions.begin();
    std::shared_ptr<Particle> neighbor = getNeighbor(*direction_iter);
    direction_iter++;
    for (; direction_iter != directions.end(); direction_iter++) {
        if (neighbor == nullptr) {
            return false;
        }
        neighbor = neighbor->getNeighbor(*direction_iter);
    }
    return neighbor != nullptr;
}

void Particle::advance() {
    FloatVector to_target = static_cast<FloatVector>(target - position);
    FloatVector to_target_pressure = to_target
                                     * (target_pressure / to_target.norm());
    // TODO make dependent on time step
    pressure += to_target_pressure;
}

Particle::Movement Particle::getMovement() const {
    Direction pressure_direction = getPressureDirection();
    return std::make_pair(pressure_direction,
                          pressure != FloatVector(0.0f, 0.0f));
}

void Particle::setTarget(const IntVector& target, float target_pressure) {
    this->target = target;
    this->target_pressure = target_pressure;
}

void Particle::move(MoveKey, Direction direction) {
    const IntVector vector = direction.vector();
    position += vector;
    // TODO Damp based on time step/distance traveled. Right now this just
    // subtracts the unit vector in movement direction.
    pressure -= static_cast<FloatVector>(vector);
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
