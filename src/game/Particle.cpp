#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(IntVector position) :
    position(position),
    target(0, 0),
    target_pressure(0.0f),
    pressure(0.0f, 0.0f),
    neighbors(),
    neighbor_connectivity() {
    setTarget(IntVector(50, 30), 1.0f); // TODO here for testing
}

const IntVector& Particle::getPosition() const {
    return position;
}

unsigned int Particle::getNumberOfNeighbors() const {
    return std::count_if(neighbors.begin(), neighbors.end(),
                         [] (std::shared_ptr<Particle> p) {
                            return p != nullptr;
                         });
}

const std::shared_ptr<Particle>& Particle::getNeighbor(Direction direction)
    const {
    // Direction is convertible to unsigned integers, starting at 0.
    return neighbors[static_cast<Direction::val_t>(direction)];
}

void Particle::setNeighbor(Direction direction,
                           const std::shared_ptr<Particle>& neighbor,
                           bool is_connected) {
    neighbors[static_cast<Direction::val_t>(direction)] = neighbor;
    if (is_connected) {
        neighbor_connectivity |= direction.bitmask();
    }
}

// Sets connectivity by asking the new neighbor (if any) whether it is connected
// via directions other than the one towards this particle.
void Particle::setNeighbor(Direction direction,
                           const std::shared_ptr<Particle>& neighbor) {
    bool is_connected = neighbor != nullptr
                        && neighbor->isConnectedViaOthers(direction.opposite());
    setNeighbor(direction, neighbor, is_connected);
}

bool Particle::isConnectedViaOthers(Direction direction) const {
    return (neighbor_connectivity & ~direction.bitmask()).any();
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

void Particle::move(Direction direction) {
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
