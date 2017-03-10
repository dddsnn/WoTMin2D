#include "ParticlePositionState.hpp"
#include "Particle.hpp"

namespace wotmin2d {

ParticlePositionState::ParticlePositionState(IntVector position) :
    position(position),
    neighbors() {
}

const IntVector& ParticlePositionState::getPosition() const {
    return position;
}

const std::shared_ptr<Particle>&
ParticlePositionState::getNeighbor(Direction direction) const {
    // Direction is convertible to unsigned integers, starting at 0.
    return neighbors[static_cast<Direction::val_t>(direction)];
}

void ParticlePositionState::setNeighbor(Direction direction,
                                        const std::shared_ptr<Particle>&
                                            neighbor) {
    neighbors[static_cast<Direction::val_t>(direction)] = neighbor;
}

bool ParticlePositionState::hasPath(std::initializer_list<Direction> directions)
    const {
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

void ParticlePositionState::move(const IntVector& vector) {
    position += vector;
}

}
