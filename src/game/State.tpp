namespace wotmin2d {

template<class P, class B>
State<P, B>::State(unsigned int arena_width, unsigned int arena_height) :
    arena_width(arena_width),
    arena_height(arena_height),
    blobs(),
    selection_center(),
    selection_radius(5.0f) {
    static_assert(Config::num_players == 1, "Can only handle exactly 1 "
                  "player.");
}

template<class P, class B>
void State<P, B>::advance(std::chrono::milliseconds time_delta) {
    for (auto& blob: blobs) {
        blob.advanceParticles(time_delta);
    }
    assert(!blobs.empty());
    std::vector<B*> blob_pointers;
    std::transform(blobs.begin(), blobs.end(),
                   std::back_inserter(blob_pointers),
                   [](B& b) { return &b; });
    std::priority_queue<B*, std::vector<B*>, BlobMobilityLess>
        blob_queue(BlobMobilityLess(), blob_pointers);
    B* current_blob = blob_queue.top();
    blob_queue.pop();
    float second_pressure;
    if (blob_queue.empty()) {
        second_pressure = -std::numeric_limits<float>::infinity();
    } else {
        assert(blob_queue.top()->getHighestMobilityParticle() != nullptr
                   && "Highest mobility particle was null.");
        second_pressure = blob_queue.top()->getHighestMobilityParticle()
                              ->getPressure().squaredNorm();
    }
    P* particle = current_blob->getHighestMobilityParticle();
    while (true) {
        assert(particle != nullptr && "Highest pressure particle in blob was "
               "null.");
        if (!particle->canMove()) {
            // Highest mobility particle can't move, nothing left to do.
            break;
        }
        handleParticle(*particle, *current_blob);
        // TODO There's optimization potential if we know that the highest
        // pressure particle will remain the highest pressure one even after
        // moving n times.
        particle = current_blob->getHighestMobilityParticle();
        if (particle->getPressure().squaredNorm() < second_pressure) {
            // After moving, the highest mobility particle of the current blob
            // has lower mobility than the blob with next highest mobility.
            // Switch blobs.
            B* new_blob = blob_queue.top();
            assert(new_blob != nullptr && "Blob was null.");
            blob_queue.pop();
            blob_queue.push(current_blob);
            current_blob = new_blob;
            particle = current_blob->getHighestMobilityParticle();
            assert(blob_queue.top()->getHighestMobilityParticle() != nullptr
                       && "Highest mobility particle was null.");
            second_pressure = blob_queue.top()->getHighestMobilityParticle()
                                  ->getPressure().squaredNorm();
        }
    }
}

template<class P, class B>
void State<P, B>::handleParticle(P& particle, B& blob) {
    Direction movement_direction = particle.getPressureDirection();
    if (isMovementOutOfBounds(particle.getPosition(), movement_direction)
        || isHostileCollision(particle, movement_direction,
                              blob.getPlayerId()))
    {
        blob.collideParticleWithWall(particle, movement_direction);
        return;
    }
    blob.handleParticle(particle, movement_direction);
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
void State<P, B>::emplaceBlob(Args... args) {
    blobs.emplace_back(std::forward<Args>(args)..., arena_width, arena_height);
}

template<class P, class B>
void State<P, B>::changeSelectionRadius(float difference) {
    selection_radius += difference;
    if (selection_radius < Config::min_selection_radius) {
        selection_radius = Config::min_selection_radius;
    }
}

template<class P, class B>
const std::vector<B>& State<P, B>::getBlobs() const {
    return blobs;
}

template<class P, class B>
void State<P, B>::selectParticles(const IntVector& center) {
    selection_center = center;
}

template<class P, class B>
void State<P, B>::setTarget(PlayerId player, const IntVector& target) {
    assert(player < Config::num_players);
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
                                     typename B::PlayerId player_id) {
    // TODO
    return false;
}

template<class P, class B>
bool State<P, B>::BlobMobilityLess::operator()(const B* first,
                                               const B* second) const {
    assert(first != nullptr);
    assert(second != nullptr);
    const P* first_particle = first->getHighestMobilityParticle();
    const P* second_particle = second->getHighestMobilityParticle();
    typename BlobState<P>::ParticleMobilityGreater pmg;
    return pmg(second_particle, first_particle);
}

}
