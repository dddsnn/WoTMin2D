namespace wotmin2d {

template<class P, class B>
State<P, B>::State(unsigned int arena_width, unsigned int arena_height) :
    arena_width(arena_width),
    arena_height(arena_height),
    blobs(),
    selection_center(),
    selection_radius(5.0f) {}

template<class P, class B>
void State<P, B>::advance(std::chrono::milliseconds time_delta) {
    for (auto& blob: blobs) {
        blob.second.advanceParticles(time_delta);
    }
    std::vector<CollidingParticle> colliding_particles;
    doParticleMovement(colliding_particles);
    resolveCollisions(colliding_particles);
}

template<class P, class B>
void State<P, B>::doParticleMovement(
    std::vector<CollidingParticle>& colliding_particles)
{
    assert(!blobs.empty());
    using IdBlob = std::pair<PlayerId, B*>;
    std::vector<IdBlob> blob_pointers;
    std::transform(blobs.begin(), blobs.end(),
                   std::back_inserter(blob_pointers),
                   [](std::pair<const PlayerId, B>& v) {
                       return std::make_pair(v.first, &v.second);
                   });
    std::priority_queue<IdBlob, std::vector<IdBlob>, BlobMobilityLess>
        blob_queue(BlobMobilityLess(), blob_pointers);
    IdBlob current_blob = blob_queue.top();
    blob_queue.pop();
    float second_pressure;
    if (blob_queue.empty()) {
        second_pressure = -std::numeric_limits<float>::infinity();
    } else {
        assert(blob_queue.top().second->getHighestMobilityParticle() != nullptr
                   && "Highest mobility particle was null.");
        second_pressure = blob_queue.top().second->getHighestMobilityParticle()
                              ->getPressure().squaredNorm();
    }
    P* particle = current_blob.second->getHighestMobilityParticle();
    while (true) {
        assert(particle != nullptr && "Highest pressure particle in blob was "
               "null.");
        if (!particle->canMove()) {
            // Highest mobility particle can't move, nothing left to do.
            break;
        }
        handleParticle(*particle, *current_blob.second, current_blob.first,
                       colliding_particles);
        // TODO There's optimization potential if we know that the highest
        // pressure particle will remain the highest pressure one even after
        // moving n times.
        particle = current_blob.second->getHighestMobilityParticle();
        if (particle->getPressure().squaredNorm() < second_pressure) {
            // After moving, the highest mobility particle of the current blob
            // has lower mobility than the blob with next highest mobility.
            // Switch blobs.
            IdBlob new_blob = blob_queue.top();
            assert(new_blob.second != nullptr && "Blob was null.");
            blob_queue.pop();
            blob_queue.push(current_blob);
            current_blob = new_blob;
            particle = current_blob.second->getHighestMobilityParticle();
            assert(blob_queue.top().second->getHighestMobilityParticle()
                       != nullptr
                       && "Highest mobility particle was null.");
            second_pressure = blob_queue.top().second
                                  ->getHighestMobilityParticle()
                                  ->getPressure().squaredNorm();
        }
    }
}

template<class P, class B>
void State<P, B>::handleParticle(
    P& particle, B& blob, PlayerId player_id,
    std::vector<CollidingParticle>& colliding_particles)
{
    Direction movement_direction = particle.getPressureDirection();
    if (isMovementOutOfBounds(particle.getPosition(), movement_direction)) {
        blob.collideParticleWithWall(particle, movement_direction);
        return;
    }
    if (isHostileCollision(particle, movement_direction, player_id)) {
        colliding_particles.emplace_back(&particle, player_id,
                                         movement_direction);
        blob.collideParticleWithWall(particle, movement_direction);
        return;
    }
    blob.handleParticle(particle, movement_direction);
}

template<class P, class B>
void State<P, B>::resolveCollisions(
    std::vector<CollidingParticle>& colliding_particles)
{
    std::unordered_set<const P*> handled_particles;
    for (auto& cp: colliding_particles) {
        P* particle;
        PlayerId player_id;
        Direction movement_direction = Direction::north();
        std::tie(particle, player_id, movement_direction) = cp;
        if (handled_particles.count(particle) > 0) {
            continue;
        }
        if (particle->getNeighbor(movement_direction) != nullptr) {
            // Particle in front is of the same blob, no action necessary.
            #ifndef NDEBUG
            IntVector forward_position = particle->getPosition()
                + movement_direction.vector();
            for (auto& id_blob: blobs) {
                assert((id_blob.first == player_id
                        || id_blob.second.getParticleAt(forward_position)
                            == nullptr)
                        && "Particles of different blobs sharing a position.");
            }
            #endif
            continue;
        }
        IntVector forward_position = particle->getPosition()
            + movement_direction.vector();
        assert(blobs.count(player_id) > 0);
        // Check if any particle of another blob is in movement direction and
        // apply damage.
        // TODO Instead of having to iterate over all blobs, it would be better
        // to have a map of all particles here. But that would require moving
        // even more functionality up into State. With a small amount of blobs I
        // guess it's fine.
        for (auto& id_blob: blobs) {
            if (id_blob.first == player_id) {
                continue;
            }
            P* forward_particle
                = id_blob.second.getParticleAt(forward_position);
            if (forward_particle == nullptr) {
                continue;
            }
            assert(blobs.count(id_blob.first) > 0);
            blobs.at(player_id).removeParticle(*particle);
            blobs.at(id_blob.first).removeParticle(*forward_particle);
            handled_particles.insert(particle);
            handled_particles.insert(forward_particle);
        }
    }
}

template<class P, class B>
unsigned int State<P, B>::getWidth() const {
    return arena_width;
}

template<class P, class B>
unsigned int State<P, B>::getHeight() const {
    return arena_height;
}

template<class P, class B>
float State<P, B>::getSelectionRadius() const {
    return selection_radius;
}

template<class P, class B>
template<class... Args>
void State<P, B>::emplaceBlob(PlayerId player_id, Args... args) {
    blobs.emplace(std::piecewise_construct,
                  std::forward_as_tuple(player_id),
                  std::forward_as_tuple(args..., arena_width, arena_height));
}

template<class P, class B>
void State<P, B>::changeSelectionRadius(float difference) {
    selection_radius += difference;
    if (selection_radius < Config::min_selection_radius) {
        selection_radius = Config::min_selection_radius;
    }
}

template<class P, class B>
const std::unordered_map<typename State<P, B>::PlayerId, B>&
State<P, B>::getBlobs() const {
    return blobs;
}

template<class P, class B>
void State<P, B>::selectParticles(const IntVector& center) {
    selection_center = center;
}

template<class P, class B>
void State<P, B>::setTarget(PlayerId player, const IntVector& target) {
    // TODO Unhardcode pressure.
    blobs[player].setTarget(target, 20.0f, selection_center, selection_radius);
}

template<class P, class B>
bool State<P, B>::isMovementOutOfBounds(const IntVector& position,
                                        Direction movement_direction) const {
    IntVector new_position = position + movement_direction.vector();
    int x = new_position.getX();
    int y = new_position.getY();
    bool x_out_of_bounds = x < 0 || static_cast<unsigned int>(x) >= arena_width;
    bool y_out_of_bounds = y < 0
         || static_cast<unsigned int>(y) >= arena_height;
    return x_out_of_bounds || y_out_of_bounds;
}

template<class P, class B>
bool State<P, B>::isHostileCollision(const P& particle,
                                     Direction movement_direction,
                                     PlayerId player_id) {
    IntVector forward_position(particle.getPosition()
                               + movement_direction.vector());
    for (const auto& id_blob: blobs) {
        if (id_blob.first == player_id) {
            continue;
        }
        const B& blob = id_blob.second;
        const P* forward_particle = blob.getParticleAt(forward_position);
        if (forward_particle != nullptr) {
            return true;
        }
    }
    return false;
}

template<class P, class B>
bool State<P, B>::BlobMobilityLess::operator()(
    const std::pair<PlayerId, B*>& first,
    const std::pair<PlayerId, B*>& second) const
{
    assert(first.second != nullptr);
    assert(second.second != nullptr);
    const P* first_particle = first.second->getHighestMobilityParticle();
    const P* second_particle = second.second->getHighestMobilityParticle();
    typename BlobState<P>::ParticleMobilityGreater pmg;
    return pmg(second_particle, first_particle);
}

}
