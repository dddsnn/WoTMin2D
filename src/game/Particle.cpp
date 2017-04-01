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

void Particle::setNeighbor(BlobStateKey, Direction direction,
                           const std::shared_ptr<Particle>& neighbor) {
    position_state.setNeighbor(direction, neighbor);
}

bool Particle::hasNeighbor() const {
    return position_state.hasNeighbor();
}

bool Particle::hasPath(std::initializer_list<Direction> directions) const {
    return position_state.hasPath(directions);
}

void Particle::advance() {
    pressure_state.advance(position_state.getPosition());
}

const FloatVector& Particle::getPressure() const {
    return pressure_state.getPressure();
}

Direction Particle::getPressureDirection() const {
    return pressure_state.getPressureDirection();
}

void Particle::setTarget(const IntVector& target, float target_pressure) {
    pressure_state.setTarget(target, target_pressure);
}

void Particle::move(BlobStateKey, Direction direction) {
    const IntVector& vector = direction.vector();
    position_state.move(vector);
    pressure_state.updatePressureAfterMovement(vector);
}

void Particle::collideWith(Particle& forward_neighbor) {
    pressure_state.collideWith(forward_neighbor.pressure_state);
}

void Particle::setFollowers(BlobStateKey,
                            const std::vector<std::shared_ptr<Particle>>&
                                followers,
                            Direction follower_direction)
{
    // TODO This is just wasteful. Use a lazy-eval adapter.
    std::vector<ParticlePressureState*> followers_pps;
    followers_pps.reserve(followers.size());
    std::transform(followers.begin(), followers.end(),
                   std::back_inserter(followers_pps),
                   [](const std::shared_ptr<Particle>& p) {
                       return &(p->pressure_state);
                   });
    pressure_state.setFollowers(followers_pps, follower_direction);
}

bool Particle::canMove() {
    // Inform pressure_state whether there the particle is blocked by a bubble,
    // i.e. there is no particle in the direction it has saved.
    Direction bubble_direction = pressure_state.getBubbleDirection();
    pressure_state.setBlocked(getNeighbor(bubble_direction) == nullptr);
    return pressure_state.canMove();
}

}
