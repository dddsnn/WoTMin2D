#include "BlobState.hpp"

namespace wotmin2d {

BlobState::BlobState() :
    particles(),
    particle_map() {
}

BlobState::~BlobState() {
    for (const Particle* particle: particles) {
        delete particle;
    }
}

const std::vector<Particle*>& BlobState::getParticles() const {
    return particles;
}

void BlobState::addParticle(const IntVector& position) {
    #ifndef NDEBUG
        ParticleMap::iterator position_iter = particle_map.find(position);
        assert(position_iter == particle_map.end()
               && "Attempt to add particle on top of another one.");
    #endif
    Particle* particle = new Particle(position);
    particles.push_back(particle);
    particle_map.emplace(position, particle);
    // Make potential neighbors aware of the new particle and vice versa.
    for (const Direction& direction: Direction::all()) {
        auto neighbor_iter = particle_map.find(position + direction.vector());
        if (neighbor_iter == particle_map.end()) {
            continue;
        }
        Particle& neighbor = *(neighbor_iter->second);
        particle->setNeighbor({}, direction, &neighbor);
        neighbor.setNeighbor({}, direction.opposite(), particle);
    }
}

void BlobState::updateParticleInformation(Particle& particle,
                                          const IntVector& old_position) {
    updateParticleMap(particle, old_position);
    updateParticleNeighbors(particle);
}

void BlobState::updateParticleMap(Particle& particle,
                                  const IntVector& old_position) {
    #ifndef NDEBUG
        ParticleMap::iterator old_position_iter
            = particle_map.find(old_position);
        assert(old_position_iter != particle_map.end()
               && old_position_iter->second == &particle
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
    particle_map.emplace(particle.getPosition(), &particle);
    assert(emplace_result.second && "There is already a particle at the new "
           "position in the map.");
}

void BlobState::updateParticleNeighbors(Particle& particle) {
    for (Direction direction: Direction::all()) {
        Particle* neighbor = particle.getNeighbor(direction);
        if (neighbor != nullptr) {
            // Unset the old neighbor's neighbor (since the particle just left).
            neighbor->setNeighbor({}, direction.opposite(), nullptr);
        }
        // Get the new neighbor from the particle map.
        // TODO Can this be done more efficiently?
        IntVector new_neighbor_position = particle.getPosition()
                                          + direction.vector();
        ParticleMap::iterator new_neighbor_iter
            = particle_map.find(new_neighbor_position);
        if (new_neighbor_iter == particle_map.end()) {
            // No neighbor in that direction at the new position.
            particle.setNeighbor({}, direction, nullptr);
        } else {
            particle.setNeighbor({}, direction, new_neighbor_iter->second);
        }
    }
    // Iterate over the neighbors of the particle again to set the particle as
    // the neighbor's neighbor. We can only do this now since we only now know
    // for sure via which directions our particle can reach the entire BlobState.
    for (Direction direction: Direction::all()) {
        Particle* neighbor = particle.getNeighbor(direction);
        if (neighbor == nullptr) {
            continue;
        }
        neighbor->setNeighbor({}, direction.opposite(), &particle);
    }
}

void BlobState::moveParticle(Particle& particle, Direction forward_direction) {
    const IntVector old_position = particle.getPosition();
    particle.move({}, forward_direction);
    updateParticleInformation(particle, old_position);
}

void BlobState::collideParticles(Particle& first, Particle& second,
                                 Direction collision_direction) {
    assert(first.getPosition().manhattanDistance(second.getPosition()) == 1
           && "Attempted to collide non-neighboring particles.");
    first.collideWith(second, collision_direction);
}

// Advances all particles to "refresh" pressure.
void BlobState::advanceParticles(std::chrono::milliseconds time_delta) {
    // TODO test
    for (Particle* particle: particles) {
        particle->advance(time_delta);
    }
}

Particle* BlobState::getHighestMobilityParticle() {
    // TODO Make this faster by using a heap somehow. The problem is only that
    // Blob may invalidate the heap and we don't know which particles it
    // touches.
    if (particles.empty()) {
        return nullptr;
    }
    return *std::max_element(particles.cbegin(), particles.cend(),
                             ParticleMobilityLess());
}

void BlobState::addParticleFollowers(Particle& leader,
                                     const std::vector<Particle*>& followers)
{
    // TODO Do I need to prevent particles from following each other?
    leader.addFollowers({}, followers);
}

// Returns whether first is less mobile than second, i.e. compares pressure and
// treats a particle whose canMove() returns false as having no pressure.
bool BlobState::ParticleMobilityLess::operator()(const Particle* first,
                                                 const Particle* second)
    const {
    bool first_can_move = first->canMove();
    bool second_can_move = second->canMove();
    if (first_can_move && second_can_move) {
        return first->getPressure().squaredNorm()
            < second->getPressure().squaredNorm();
    } else if (second_can_move) {
        return true;
    } else {
        return false;
    }
}

}
