namespace wotmin2d {

template<class P>
BlobState<P>::BlobState() :
    particles(),
    particle_map() {
}

template<class P>
BlobState<P>::~BlobState() {
    for (const P* particle: particles) {
        delete particle;
    }
}

template<class P>
const typename BlobState<P>::ParticleSet& BlobState<P>::getParticles() const {
    return particles;
}

template<class P>
const std::vector<P*> BlobState<P>::getParticles(const IntVector& center,
                                                 float radius) const {
    // TODO If particle_map were two nested one-dimensional maps, this could go
    // faster (just using lower_bound() and upper_bound()).
    if (radius < 0.0f) {
        return {};
    }
    std::vector<P*> particles;
    // Add center already. We can't do that in the loop, or else we'd add it
    // four times.
    auto center_iter = particle_map.find(center);
    if (center_iter != particle_map.end()) {
        particles.push_back(center_iter->second);
    }
    int radius_int = static_cast<int>(radius);
    int squared_radius = static_cast<int>(radius * radius);
    int half_width = 1;
    for (int i = radius_int; i > 0; i--) {
        for (; IntVector(half_width, i).squaredNorm() <= squared_radius;
             half_width++);
        // Look in all four cardinal directions, up to the radius.
        for (Direction forward: Direction::all()) {
            // From there, turn right, up to the width of the circle at the
            // distance from the center.
            for (int j = 0; j < half_width; j++) {
                IntVector position = center + (forward.vector() * i)
                                     + (forward.right().vector() * j);
                auto iter = particle_map.find(position);
                if (iter != particle_map.end()) {
                    particles.push_back(iter->second);
                }
            }
        }
    }
    return particles;
}

template<class P>
const P* BlobState<P>::getParticleAt(const IntVector& position) const {
    auto iter = particle_map.find(position);
    if (iter == particle_map.end()) {
        return nullptr;
    }
    return iter->second;
}

template<class P>
void BlobState<P>::addParticle(const IntVector& position) {
    #ifndef NDEBUG
        typename ParticleMap::iterator position_iter
            = particle_map.find(position);
        assert(position_iter == particle_map.end()
               && "Attempt to add particle on top of another one.");
    #endif
    P* particle = new P(position);
    particles.insert(particle);
    particle_map.emplace(position, particle);
    // Make potential neighbors aware of the new particle and vice versa.
    for (const Direction& direction: Direction::all()) {
        auto neighbor_iter = particle_map.find(position + direction.vector());
        if (neighbor_iter == particle_map.end()) {
            continue;
        }
        P& neighbor = *(neighbor_iter->second);
        particle->setNeighbor({}, direction, &neighbor);
        neighbor.setNeighbor({}, direction.opposite(), particle);
    }
}

template<class P>
void BlobState<P>::updateParticleInformation(P& particle,
                                             const IntVector& old_position) {
    updateParticleMap(particle, old_position);
    updateParticleNeighbors(particle);
}

template<class P>
void BlobState<P>::updateParticleMap(P& particle,
                                     const IntVector& old_position) {
    #ifndef NDEBUG
        typename ParticleMap::iterator old_position_iter
            = particle_map.find(old_position);
        assert(old_position_iter != particle_map.end()
               && old_position_iter->second == &particle
               && "P not found at the position it's supposed to be.");
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

template<class P>
void BlobState<P>::updateParticleNeighbors(P& particle) {
    for (Direction direction: Direction::all()) {
        P* neighbor = particle.getNeighbor(direction);
        if (neighbor != nullptr) {
            // Unset the old neighbor's neighbor (since the particle just left).
            neighbor->setNeighbor({}, direction.opposite(), nullptr);
        }
        // Get the new neighbor from the particle map.
        // TODO Can this be done more efficiently?
        IntVector new_neighbor_position = particle.getPosition()
                                          + direction.vector();
        typename ParticleMap::iterator new_neighbor_iter
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
    // for sure via which directions our particle can reach the entire BlobState<P>.
    for (Direction direction: Direction::all()) {
        P* neighbor = particle.getNeighbor(direction);
        if (neighbor == nullptr) {
            continue;
        }
        neighbor->setNeighbor({}, direction.opposite(), &particle);
    }
}

template<class P>
void BlobState<P>::moveParticle(P& particle, Direction forward_direction) {
    const IntVector old_position = particle.getPosition();
    auto modifier = [=](P* p) { p->move({}, forward_direction); };
    modifyParticle(particle, modifier);
    updateParticleInformation(particle, old_position);
}

template<class P>
void BlobState<P>::collideParticles(P& first, P& second,
                                    Direction collision_direction) {
    assert(first.getPosition().manhattanDistance(second.getPosition()) == 1
           && "Attempted to collide non-neighboring particles.");
    assert(collision_direction.vector()
               == second.getPosition() - first.getPosition()
           && "Collision direction doesn't correspond to relative particle "
              "positions.");
    auto modifier = [&second, collision_direction](P* p) {
        p->collideWith({}, second, collision_direction);
    };
    modifyParticle(first, modifier);
    // Modify second as well so it's put into its possibly changed place in the
    // index (modification itself is a nop). The order of particles in the 0-th
    // index never changes, so this is safe.
    modifyParticle(second, [](P*){ /* nop */ });
}

template<class P>
void BlobState<P>::collideParticleWithWall(P& particle,
                                           Direction collision_direction) {
    auto modifier = [=](P* p) {
        p->killPressureInDirection({}, collision_direction);
    };
    modifyParticle(particle, modifier);
}

// Advances all particles to "refresh" pressure.
template<class P>
void BlobState<P>::advanceParticles(std::chrono::milliseconds time_delta) {
    auto modifier = [=](P* p) { p->advance({}, time_delta); };
    // Modifying particles doesn't change their order in the 0-th index (using
    // identity, i.e. their memory address, as key), so the iterator stays valid
    // even if we modify stuff.
    for (P* particle: particles) {
        modifyParticle(*particle, modifier);
    }
}

template<class P>
P* BlobState<P>::getHighestMobilityParticle() {
    if (particles.empty()) {
        return nullptr;
    }
    const typename ParticleSet::template nth_index<1>::type& mobility_index
        = particles.template get<1>();
    return *(mobility_index.begin());
}

template<class P>
void BlobState<P>::addParticleFollowers(P& leader,
                                        const std::vector<P*>& followers) {
    // TODO Do I need to prevent particles from following each other?
    auto modifier = [&](P* p) { p->addFollowers({}, followers); };
    modifyParticle(leader, modifier);
    particles.insert(&leader);
    for (P* follower: followers) {
        // Do a nop-modification on all followers so they're reordered in the
        // mobility index.
        modifyParticle(*follower, [](P*){});
    }
}

template<class P>
template<class Modifier>
void BlobState<P>::modifyParticle(P& particle, Modifier modifier) {
    typename ParticleSet::iterator it = particles.find(&particle);
    assert(it != particles.end());
    #ifndef NDEBUG
    bool success =
    #endif
    particles.modify(it, modifier);
    assert(success && "Modification of particle failed.");
}

// Returns whether first is more mobile than second, i.e. compares pressure and
// treats a particle whose canMove() returns false as having no pressure.
template<class P>
bool BlobState<P>::ParticleMobilityGreater::operator()(const P* first,
                                                       const P* second) const {
    assert(first != nullptr);
    assert(second != nullptr);
    bool first_can_move = first->canMove();
    bool second_can_move = second->canMove();
    if (first_can_move && second_can_move) {
        return first->getPressure().squaredNorm()
            > second->getPressure().squaredNorm();
    } else if (first_can_move) {
        return true;
    } else {
        return false;
    }
}

}
