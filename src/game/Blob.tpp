namespace wotmin2d {

template<class T>
using Ptr = std::shared_ptr<T>;

template<class B, class P>
Blob<B, P>::Blob(unsigned int arena_width, unsigned int arena_height,
                 std::shared_ptr<B> state) :
    state(state),
    arena_width(arena_width),
    arena_height(arena_height) {
}

template<class B, class P>
Blob<B, P>::Blob(const IntVector& center, float radius, unsigned int arena_width,
                 unsigned int arena_height, std::shared_ptr<B> state) :
    state(state),
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
                state->addParticle(coordinate);
            }
        }
    }
}

template<class B, class P>
const std::vector<Ptr<P>>& Blob<B, P>::getParticles() const {
    return state->getParticles();
}

template<class B, class P>
void Blob<B, P>::advance() {
    for (const Ptr<P>& particle: state->getParticles()) {
        assert(particle != nullptr && "Particle in blob was null.");
        advanceParticle(particle);
    }
}

template<class B, class P>
void Blob<B, P>::advanceParticle(const Ptr<P>& particle) {
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
    const Ptr<P>& forward_neighbor
        = particle->getNeighbor(forward_direction);
    if (forward_neighbor != nullptr) {
        // Movement is obstructed. Only collide.
        particle->collideWith(*forward_neighbor);
        return;
    }
    moveParticleLine(particle, forward_direction);
}

// Moves a particle and drags all particles behind it in the same direction so
// that no bubbles appear and the blob stays connected.
template<class B, class P>
void Blob<B, P>::moveParticleLine(Ptr<P> first_particle,
                                  Direction forward_direction) {
    while (true) {
        Ptr<P> next_particle
            = first_particle->getNeighbor(forward_direction.opposite());
        if (next_particle == nullptr) {
            // We're at the end of the line, first_particle is now the last
            // particle. Check whether we're isolating particles to the side of
            // it and drag them in behind if that's the case.
            // We have to get both side neighbors before we can move the
            // particle so it still has its old neighbors.
            using ParticleDirection = std::pair<Ptr<P>, Direction>;
            Direction left = forward_direction.left();
            Direction right = forward_direction.right();
            std::array<ParticleDirection, 2> side_neighbors_directions
                = { std::make_pair(first_particle->getNeighbor(left), left),
                    std::make_pair(first_particle->getNeighbor(right), right) };
            // Now we can move it.
            state->moveParticle(first_particle, forward_direction);
            for (const ParticleDirection& side_neighbor_direction:
                 side_neighbors_directions)
            {
                const Ptr<P>& side_neighbor
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
        // TODO Implement a method in BlobState to move an entire line without
        // updating particle information in the map for each individually.
        state->moveParticle(first_particle, forward_direction);
        // Swap and proceed with the next particle in line.
        using std::swap;
        swap(first_particle, next_particle);
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
template<class B, class P>
bool Blob<B, P>::dragParticlesBehindLine(Ptr<P> particle,
                                         Direction forward_direction,
                                         Direction line_direction) {
    bool has_moved = false;
    std::unordered_set<Ptr<P>> done_set;
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
        Ptr<P> next_particle;
        Direction next_direction = Direction::north();
        for (Direction direction: forward_direction.others()) {
            // Direction::others() returns directions always counter-clockwise
            // to the forward_direction, so there shouldn't be an infinite loop
            // since we always prefer to go to the left first, then back and
            // only then to the right.
            const Ptr<P>& neighbor = particle->getNeighbor(direction);
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
        state->moveParticle(particle, forward_direction);
        has_moved = true;
        // Set particle to the next particle.
        particle = std::move(next_particle);
        line_direction = forward_direction;
        forward_direction = next_direction;
    }
    return has_moved;
}

}
