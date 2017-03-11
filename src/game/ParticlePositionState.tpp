namespace wotmin2d {

template<class P>
ParticlePositionState<P>::ParticlePositionState(IntVector position) :
    position(position),
    neighbors() {
}

template<class P>
const IntVector& ParticlePositionState<P>::getPosition() const {
    return position;
}

template<class P>
std::shared_ptr<P>
ParticlePositionState<P>::getNeighbor(Direction direction) const {
    // Direction is convertible to unsigned integers, starting at 0.
    return neighbors[static_cast<Direction::val_t>(direction)].lock();
}

template<class P>
void ParticlePositionState<P>::setNeighbor(Direction direction,
                                           const std::shared_ptr<P>& neighbor) {
    neighbors[static_cast<Direction::val_t>(direction)] = neighbor;
}

template<class P>
bool ParticlePositionState<P>::hasPath(std::initializer_list<Direction>
    directions) const {
    if (directions.size() == 0) {
        return true;
    }
    auto direction_iter = directions.begin();
    std::shared_ptr<P> neighbor = getNeighbor(*direction_iter);
    direction_iter++;
    for (; direction_iter != directions.end(); direction_iter++) {
        if (neighbor == nullptr) {
            return false;
        }
        neighbor = neighbor->getNeighbor(*direction_iter);
    }
    return neighbor != nullptr;
}

template<class P>
void ParticlePositionState<P>::move(const IntVector& vector) {
    position += vector;
}

}
