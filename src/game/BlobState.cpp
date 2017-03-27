#include "BlobState.hpp"

namespace wotmin2d {

using ParticlePtr = std::shared_ptr<Particle>;

BlobState::BlobState() :
    particles(),
    particle_map() {
}

const std::vector<ParticlePtr>& BlobState::getParticles() const {
    return particles;
}

void BlobState::addParticle(const IntVector& position) {
    // TODO Check that there isn't a particle at the position already.
    ParticlePtr particle = std::make_shared<Particle>(position);
    particles.push_back(particle);
    particle_map.emplace(position, particle);
    // Make potential neighbors aware of the new particle and vice versa.
    for (const Direction& direction: Direction::all()) {
        auto neighbor_iter = particle_map.find(position + direction.vector());
        if (neighbor_iter == particle_map.end()) {
            continue;
        }
        ParticlePtr& neighbor = neighbor_iter->second;
        particle->setNeighbor({}, direction, neighbor);
        neighbor->setNeighbor({}, direction.opposite(), particle);
    }
}

void BlobState::updateParticleInformation(const ParticlePtr& particle,
                                          const IntVector& old_position) {
    updateParticleMap(particle, old_position);
    updateParticleNeighbors(particle);
}

void BlobState::updateParticleMap(const ParticlePtr& particle,
                             const IntVector& old_position) {
    #ifndef NDEBUG
        ParticleMap::iterator old_position_iter
            = particle_map.find(old_position);
        assert(old_position_iter != particle_map.end()
               && old_position_iter->second == particle
               && "Particle not found at the position it's supposed to be.");
    #endif
    // Delete particle at old position.
    particle_map.erase(old_position);
    // Put back in at the new position. We're deleting a shared pointer here and
    // immediately copying it back in. I haven't found a better solution without
    // using C++17 features.
    #ifndef NDEBUG
        auto emplace_result =
    #endif
    particle_map.emplace(particle->getPosition(), particle);
    assert(emplace_result.second && "There is already a particle at the new "
           "position in the map.");
}

void BlobState::updateParticleNeighbors(const ParticlePtr& particle) {
    for (Direction direction: Direction::all()) {
        const ParticlePtr& neighbor
            = particle->getNeighbor(direction);
        if (neighbor != nullptr) {
            // Unset the old neighbor's neighbor (since the particle just left).
            neighbor->setNeighbor({}, direction.opposite(), nullptr);
        }
        // Get the new neighbor from the particle map.
        // TODO Can this be done more efficiently?
        IntVector new_neighbor_position = particle->getPosition()
                                          + direction.vector();
        ParticleMap::iterator new_neighbor_iter
            = particle_map.find(new_neighbor_position);
        if (new_neighbor_iter == particle_map.end()) {
            // No neighbor in that direction at the new position.
            particle->setNeighbor({}, direction, nullptr);
        } else {
            particle->setNeighbor({}, direction, new_neighbor_iter->second);
        }
    }
    // Iterate over the neighbors of the particle again to set the particle as
    // the neighbor's neighbor. We can only do this now since we only now know
    // for sure via which directions our particle can reach the entire BlobState.
    for (Direction direction: Direction::all()) {
        const ParticlePtr& neighbor
            = particle->getNeighbor(direction);
        if (neighbor == nullptr) {
            continue;
        }
        neighbor->setNeighbor({}, direction.opposite(), particle);
    }
}

void BlobState::moveParticle(const ParticlePtr& first_particle,
                             Direction forward_direction) {
    const IntVector old_position = first_particle->getPosition();
    first_particle->move({}, forward_direction);
    updateParticleInformation(first_particle, old_position);
}

void BlobState::collideParticles(const ParticlePtr& first,
                                 const ParticlePtr& second) {
    first->collideWith(*second);
}

}
