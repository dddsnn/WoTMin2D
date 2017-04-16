namespace wotmin2d {

template<class P, class B>
Blob<P, B>::Blob(PlayerId player_id, unsigned int arena_width,
                 unsigned int arena_height, std::shared_ptr<B> state) :
    player_id(player_id),
    state(state),
    arena_width(arena_width),
    arena_height(arena_height)
{
    assert(player_id < Config::num_players);
}

template<class P, class B>
Blob<P, B>::Blob(PlayerId player_id, const IntVector& center, float radius,
                 unsigned int arena_width, unsigned int arena_height,
                 std::shared_ptr<B> state) :
    player_id(player_id),
    state(state),
    arena_width(arena_width),
    arena_height(arena_height)
{
    assert(player_id < Config::num_players);
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
            int squaredNorm = center_to_coordinate.squaredNorm();
            if (squaredNorm <= squared_radius) {
                state->addParticle(coordinate);
            }
        }
    }
}

template<class P, class B>
const typename BlobState<P>::ParticleSet& Blob<P, B>::getParticles() const {
    return state->getParticles();
}

template<class P, class B>
void Blob<P, B>::advance(std::chrono::milliseconds time_delta) {
    state->advanceParticles(time_delta);
    while (true) {
        // TODO There's optimization potential if we know that the highest
        // pressure particle will remain the highest pressure one even after
        // moving n times.
        P* particle = state->getHighestMobilityParticle();
        assert(particle != nullptr && "Highest pressure particle in blob was "
               "null.");
        if (!particle->canMove()) {
            // Highest mobility particle can't move, nothing left to do.
            break;
        }
        handleParticle(*particle);
    }
}

template<class P, class B>
void Blob<P, B>::setTarget(const IntVector& target, float pressure_per_second,
                           const IntVector& center, float radius) {
    const std::vector<P*> particles = state->getParticles(center, radius);
    for (P* particle: particles) {
        particle->setTarget(target, pressure_per_second);
    }
}

template<class P, class B>
void Blob<P, B>::handleParticle(P& particle) {
    Direction movement_direction = particle.getPressureDirection();
    if (isMovementOutOfBounds(particle.getPosition(), movement_direction)) {
        state->collideParticleWithWall(particle, movement_direction);
        return;
    }
    P* forward_neighbor = particle.getNeighbor(movement_direction);
    if (forward_neighbor != nullptr) {
        // Movement is obstructed. Only collide.
        state->collideParticles(particle, *forward_neighbor,
                                movement_direction);
        return;
    }
    // TODO Maybe add more than the immediate neighbors? Maybe also their
    // neighbors?
    std::vector<P*> neighbors;
    neighbors.reserve(3);
    for (Direction direction: movement_direction.others()) {
        P* neighbor = particle.getNeighbor(direction);
        if (neighbor != nullptr) {
            neighbors.push_back(neighbor);
        }
    }
    state->moveParticle(particle, movement_direction);
    if (!particle.hasNeighbor()) {
        // We've disconnected the particle by moving, make the previous
        // neighbors follow it to catch up.
        state->addParticleFollowers(particle, neighbors);
    }
}

template<class P, class B>
bool Blob<P, B>::isMovementOutOfBounds(const IntVector& position,
                                       Direction movement_direction) const {
    IntVector new_position = position + movement_direction.vector();
    int x = new_position.getX();
    int y = new_position.getY();
    bool x_out_of_bounds = x < 0 || static_cast<unsigned int>(x) >= arena_width;
    bool y_out_of_bounds = y < 0
         || static_cast<unsigned int>(y) >= arena_height;
    return x_out_of_bounds || y_out_of_bounds;
}

}
