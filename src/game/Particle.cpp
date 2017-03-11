#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(IntVector position) :
    position_state(position),
    pressure_state() {
    pressure_state.setTarget(IntVector(50, 30), 1.0f); // TODO here for testing
}

const IntVector& Particle::getPosition() const {
    return position_state.getPosition();
}

std::shared_ptr<Particle> Particle::getNeighbor(Direction direction) const {
    return position_state.getNeighbor(direction);
}

void Particle::setNeighbor(MoveKey, Direction direction,
                           const std::shared_ptr<Particle>& neighbor) {
    position_state.setNeighbor(direction, neighbor);
}

bool Particle::hasPath(std::initializer_list<Direction> directions) const {
    return position_state.hasPath(directions);
}

void Particle::advance() {
    pressure_state.advance(position_state.getPosition());
}

Particle::Movement Particle::getMovement() const {
    return pressure_state.getMovement();
}

void Particle::setTarget(const IntVector& target, float target_pressure) {
    pressure_state.setTarget(target, target_pressure);
}

void Particle::move(MoveKey, Direction direction) {
    const IntVector& vector = direction.vector();
    position_state.move(vector);
    pressure_state.updatePressureAfterMovement(vector);
}

void Particle::collideWith(Particle& forward_neighbor) {
    ParticlePressureState& forward_neighbor_pressure_state
        = forward_neighbor.pressure_state;
    pressure_state.collideWith(forward_neighbor_pressure_state);
}

}
