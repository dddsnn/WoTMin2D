#include "Blob.hpp"

namespace wotmin2d {

Blob::Blob(const unsigned int& arena_width, const unsigned int& arena_height) :
    particles(),
    particle_map(),
    arena_width(arena_width),
    arena_height(arena_height) {
}

Blob::Blob(const IntVector& center, float radius,
           const unsigned int& arena_width, const unsigned int& arena_height) :
    particles(),
    particle_map(),
    arena_width(arena_width),
    arena_height(arena_height) {
    if (radius <= 0.0) {
        return;
    }
    // Add particles by checking every vector within the bounding box of the
    // circle whether it's less than the radius from the center.
    int rounded_radius = static_cast<int>(radius + 0.5);
    int left = std::max(0, center.getX() - rounded_radius);
    int top = std::max(0, center.getY() - rounded_radius);
    int right = std::min(center.getX() + rounded_radius,
                         static_cast<int>(arena_width - 1));
    int bottom = std::min(center.getY() + rounded_radius,
                          static_cast<int>(arena_height - 1));
    float squared_radius = radius * radius;
    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            IntVector coordinate(x, y);
            IntVector center_to_coordinate = coordinate - center;
            int dot_product = center_to_coordinate.dot(center_to_coordinate);
            if (dot_product < squared_radius) {
                addParticleConnected(coordinate);
            }
        }
    }
}

const std::vector<std::shared_ptr<Particle>>& Blob::getParticles() const {
    return particles;
}

// Sets all connectivity flags to true. This is only correct if the reachability
// graph (via the neighbor pointers) is 2-connected.
void Blob::addParticleConnected(const IntVector& position) {
    std::shared_ptr<Particle> particle = std::make_shared<Particle>(position);
    particles.push_back(particle);
    particle_map.emplace(position, particle);
    // Make potential neighbors aware of the new particle and vice versa.
    for (const Direction& direction: Direction::all()) {
        auto neighbor_iter = particle_map.find(position + direction.vector());
        if (neighbor_iter == particle_map.end()) {
            continue;
        }
        std::shared_ptr<Particle>& neighbor = neighbor_iter->second;
        // Just set connectivity to true for everything, since the blob is
        // 2-connected.
        particle->setNeighbor(direction, neighbor, true);
        neighbor->setNeighbor(direction.opposite(), particle, true);
    }
}

void Blob::advance() {
    for (const std::shared_ptr<Particle>& particle: particles) {
        assert(particle != nullptr && "Particle in blob was null.");
        advanceParticle(particle);
    }
}

void Blob::advanceParticle(const std::shared_ptr<Particle>& particle) {
    // Advance particle to "refresh" pressure.
    particle->advance();
    // Now, where does it want to go?
    using Movement = Particle::Movement;
    Movement movement = particle->getMovement();
    if (!movement.second) {
        // Particle doesn't want to move.
        return;
    }
    const Direction& forward_direction = movement.first;
    const std::shared_ptr<Particle>& forward_neighbor
        = particle->getNeighbor(forward_direction);
    if (forward_neighbor != nullptr) {
        // Movement is obstructed. Only collide.
        particle->collideWith(*forward_neighbor);
        return;
    }
    moveParticleLine(particle, forward_direction);
}

void Blob::updateParticleInformation(const std::shared_ptr<Particle>& particle,
                                     const IntVector& old_position) {
    updateParticleMap(particle, old_position);
    updateParticleNeighbors(particle);
}

void Blob::updateParticleMap(const std::shared_ptr<Particle>& particle,
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
    auto emplace_result = particle_map.emplace(particle->getPosition(),
                                               particle);
    assert(emplace_result.second && "There is already a particle at the new "
           "position in the map.");
}

void Blob::updateParticleNeighbors(const std::shared_ptr<Particle>& particle) {
    for (Direction direction: Direction::all()) {
        const std::shared_ptr<Particle>& neighbor
            = particle->getNeighbor(direction);
        if (neighbor != nullptr) {
            // Unset the old neighbor's neighbor (since the particle just left).
            neighbor->setNeighbor(direction.opposite(), nullptr);
        }
        // Get the new neighbor from the particle map.
        // TODO Can this be done more efficiently?
        IntVector new_neighbor_position = particle->getPosition()
                                          + direction.vector();
        ParticleMap::iterator new_neighbor_iter
            = particle_map.find(new_neighbor_position);
        if (new_neighbor_iter == particle_map.end()) {
            // No neighbor in that direction at the new position.
            particle->setNeighbor(direction, nullptr);
        } else {
            particle->setNeighbor(direction, new_neighbor_iter->second);
        }
    }
    // Iterate over the neighbors of the particle again to set the particle as
    // the neighbor's neighbor. We can only do this now since we only now know
    // for sure via which directions our particle can reach the entire blob.
    for (Direction direction: Direction::all()) {
        const std::shared_ptr<Particle>& neighbor
            = particle->getNeighbor(direction);
        if (neighbor == nullptr) {
            continue;
        }
        neighbor->setNeighbor(direction.opposite(), particle);
    }
}

// Moves a particle and drags all particles behind it in the same direction so
// that no bubbles appear.
void Blob::moveParticleLine(std::shared_ptr<Particle> first_particle,
                            Direction forward_direction) {
    std::array<Direction, 2> side_directions = { forward_direction.left(),
                                                 Direction::north()
                                               };
    // TODO add Direction::right() to avoid this.
    side_directions[1] = side_directions[0].opposite();
    while (true) {
        const IntVector old_position = first_particle->getPosition();
        std::shared_ptr<Particle> next_particle
            = first_particle->getNeighbor(forward_direction.opposite());
        first_particle->move(forward_direction);
        if (next_particle == nullptr) {
            // We're at the end of the line, first_particle is now the last
            // particle. Check whether we're isolating particles to the side of
            // it and pull them in behind is that's the case.
            // We have to do this before we call updateParticleInformation() for
            // first_particle so it still has its old neighbors.
            for (Direction side_direction: side_directions) {
                const std::shared_ptr<Particle> side_neighbor
                    = first_particle->getNeighbor(side_direction);
                if (side_neighbor == nullptr
                    || side_neighbor->getNumberOfNeighbors() > 1) {
                    continue;
                }
                const IntVector side_old_position
                    = side_neighbor->getPosition();
                side_neighbor->move(side_direction.opposite());
                // Update first_particle's information first so it's not in the
                // way in the particle map.
                updateParticleInformation(first_particle, old_position);
                updateParticleInformation(side_neighbor, side_old_position);
                // We've moved one particle, no need to also move the other one,
                // since it can't be disconnected anymore (it's adjacent to the
                // particle we just moved).
                return;
            }
            // At this point, neither of the side neighbors had to be moved. But
            // we still have to update first_particle's information since we
            // couldn't do it before.
            updateParticleInformation(first_particle, old_position);
            return;
        }
        // TODO Could be done more efficiently in one go instead of separately
        // for each changed particle.
        updateParticleInformation(first_particle, old_position);
        // Swap and proceed with the next particle in line.
        std::swap(first_particle, next_particle);
    }
}

}
