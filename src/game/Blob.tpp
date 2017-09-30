namespace wotmin2d {

template<class P, class B>
Blob<P, B>::Blob(PlayerId player_id, std::shared_ptr<B> state) :
    player_id(player_id),
    state(state)
{
    assert(player_id < Config::num_players);
}

template<class P, class B>
Blob<P, B>::Blob(PlayerId player_id, const IntVector& center, float radius,
                 unsigned int arena_width, unsigned int arena_height,
                 std::shared_ptr<B> state) :
    player_id(player_id),
    state(state)
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
typename Blob<P, B>::PlayerId Blob<P, B>::getPlayerId() const {
    return player_id;
}

template<class P, class B>
void Blob<P, B>::advanceParticles(std::chrono::milliseconds time_delta) {
    state->advanceParticles(time_delta);
}

template<class P, class B>
P* Blob<P, B>::getHighestMobilityParticle() const {
    return state->getHighestMobilityParticle();
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
void Blob<P, B>::collideParticleWithWall(P& particle,
                                         Direction collision_direction) {
    state->collideParticleWithWall(particle, collision_direction);
}

template<class P, class B>
void Blob<P, B>::handleParticle(P& particle, Direction movement_direction) {
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

}
