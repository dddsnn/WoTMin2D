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
        particle->setNeighbor(direction, neighbor);
        neighbor->setNeighbor(direction.opposite(), particle);
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
    // TODO Should I advance() all particles before I move any of them? Moving
    // one can entail moving another in order to avoid bubbles/disconnection.
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
// that no bubbles appear and the blob stays connected.
void Blob::moveParticleLine(std::shared_ptr<Particle> first_particle,
                            Direction forward_direction) {
    while (true) {
        const IntVector old_position = first_particle->getPosition();
        std::shared_ptr<Particle> next_particle
            = first_particle->getNeighbor(forward_direction.opposite());
        first_particle->move(forward_direction);
        if (next_particle == nullptr) {
            // We're at the end of the line, first_particle is now the last
            // particle. Check whether we're isolating particles to the side of
            // it and drag them in behind if that's the case.
            // We have to get both side neighbors before we call
            // updateParticleInformation() for first_particle so it still has
            // its old neighbors.
            using ParticleDirection
                = std::pair<std::shared_ptr<Particle>, Direction>;
            Direction left = forward_direction.left();
            Direction right = forward_direction.right();
            std::array<ParticleDirection, 2> side_neighbors_directions
                = { std::make_pair(first_particle->getNeighbor(left), left),
                    std::make_pair(first_particle->getNeighbor(right), right) };
            // Now we can update.
            updateParticleInformation(first_particle, old_position);
            for (const ParticleDirection& side_neighbor_direction:
                 side_neighbors_directions)
            {
                const std::shared_ptr<Particle>& side_neighbor
                    = side_neighbor_direction.first;
                const Direction side_direction = side_neighbor_direction.second;
                if (side_neighbor == nullptr) {
                    continue;
                }
                // Drag the side neighbor to first_particle's old position if
                // necessary.
                if (dragParticlesBehindLine(std::move(side_neighbor),
                                            side_direction.opposite(),
                                            forward_direction)) {
                    // We've moved one particle, no need to also move the other
                    // one, since it can't be disconnected anymore (it's
                    // adjacent to the particle we just moved).
                    return;
                } else {
                    // The neighbor didn't need to be moved (since it still had
                    // connectivity). We need to continue and potentially check
                    // the other neighbor.
                    continue;
                }
            }
            // Neither of the side neighbors had to be moved, we're done.
            return;
        }
        // TODO Could be done more efficiently in one go instead of separately
        // for each changed particle.
        updateParticleInformation(first_particle, old_position);
        // Swap and proceed with the next particle in line.
        std::swap(first_particle, next_particle);
    }
}

// Moves particle one in forward_direction if necessary to preserve local
// connectivity, and then drags particles after it if necessary to preserve
// connectivity.
// To be used after a particle (or a line of particles) has been moved in
// line_direction to preserve connectivity. The position in forward_direction of
// particle must be empty (it's the one where the particle that has been moved
// was before).
// particle is said to have local connectivity if it has a neighbor in
// line_direction, which has the particle that was moved as a neighbor in
// forward_direction.
// Returns whether any particle was moved.
bool Blob::dragParticlesBehindLine(std::shared_ptr<Particle> particle,
                                   Direction forward_direction,
                                   Direction line_direction) {
    bool has_moved = false;
    std::unordered_set<std::shared_ptr<Particle>> done_set;
    // TODO Does this always terminate?
    while (particle != nullptr) {
        done_set.insert(particle);
        // Figure out if we even need to move or if there is local connectivity
        // to the line.
        bool need_to_move;
        if (forward_direction != line_direction) {
            need_to_move = !particle->hasPath({ line_direction,
                                                forward_direction });
        } else {
            // The direction we have to move is the same as the one the previous
            // particle moved. If we can jump the gap to one side we're don't
            // have to move, but if we're covered on both we do or else we would
            // create a bubble.
            bool has_left = particle->hasPath({ line_direction.left(),
                                                line_direction,
                                                line_direction,
                                                line_direction.right() });
            bool has_right = particle->hasPath({ line_direction.right(),
                                                 line_direction,
                                                 line_direction,
                                                 line_direction.left() });
            need_to_move = has_left == has_right;
        }
        if (!need_to_move) {
            return has_moved;
        }
        // Check if there is a next particle that might also be disconnected.
        std::shared_ptr<Particle> next_particle;
        Direction next_direction = Direction::north();
        for (Direction direction: forward_direction.others()) {
            // TODO Test that Direction::others() returns directions
            // counter-clockwise.
            // Direction::others() returns directions always counter-clockwise
            // to the forward_direction, so there shouldn't be an infinite loop
            // since we always prefer to go to the left first, then back and
            // only then to the right.
            const std::shared_ptr<Particle>& neighbor
                = particle->getNeighbor(direction);
            if (neighbor == nullptr) {
                continue;
            }
            if (done_set.count(neighbor) > 0) {
                // The neighbor is one that was already moved by this function.
                // Don't consider it for moving, as that would lead to an
                // infinite loop. It should have connectivity (though not
                // necessarily local) because this should only happen if there
                // is a bubble that we've just traversed the outside of.
                continue;
            }
            next_particle = neighbor;
            next_direction = direction.opposite();
            break;
        }
        // Move the current particle.
        const IntVector old_position = particle->getPosition();
        particle->move(forward_direction);
        updateParticleInformation(particle, old_position);
        has_moved = true;
        // Set particle to the next particle.
        particle = std::move(next_particle);
        line_direction = forward_direction;
        forward_direction = next_direction;
    }
    return has_moved;
}

}
