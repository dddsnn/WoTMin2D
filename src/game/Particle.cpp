#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(IntVector position) :
    position(position),
    neighbors(),
    target(0, 0),
    target_pressure_per_second(0.0f),
    pressure(0.0f, 0.0f),
    followers(),
    leaders() {
    setTarget(IntVector(50, 30), 20.0f); // TODO here for testing
}

const IntVector& Particle::getPosition() const {
    return position;
}

Particle* Particle::getNeighbor(Direction direction) {
    // Direction is convertible to unsigned integers, starting at 0.
    return neighbors[static_cast<Direction::val_t>(direction)];
}

const Particle* Particle::getConstNeighbor(Direction direction) const {
    return neighbors[static_cast<Direction::val_t>(direction)];
}

void Particle::setNeighbor(BlobStateKey, Direction direction,
                           Particle* neighbor) {
    neighbors[static_cast<Direction::val_t>(direction)] = neighbor;
}

bool Particle::hasNeighbor() const {
    return std::any_of(neighbors.begin(), neighbors.end(),
                       [](const Particle* p) { return p != nullptr; });
}

bool Particle::hasPath(std::initializer_list<Direction> directions) const {
    if (directions.size() == 0) {
        return true;
    }
    auto direction_iter = directions.begin();
    const Particle* neighbor = getConstNeighbor(*direction_iter);
    direction_iter++;
    for (; direction_iter != directions.end(); direction_iter++) {
        if (neighbor == nullptr) {
            return false;
        }
        neighbor = neighbor->getConstNeighbor(*direction_iter);
    }
    return neighbor != nullptr;
}

void Particle::advance(std::chrono::milliseconds time_delta) {
    std::chrono::duration<float, std::ratio<1>> second_fraction = time_delta;
    float target_pressure = second_fraction.count()
                            * target_pressure_per_second;
    float divisor = static_cast<float>(followers.size() + 1);
    float pressure_part = target_pressure / divisor;
    FloatVector to_target = static_cast<FloatVector>(target - position);
    FloatVector to_target_pressure = to_target
                                     * (pressure_part / to_target.norm());
    pressure += to_target_pressure;
    addPressureToFollowers(followers, pressure_part);
    // TODO Calling this here means that some of the followers/leaders will not
    // yet have their pressures updated (as it hasn't been their turn to be
    // updated). This isn't ideal, but doing it properly means having BlobState
    // iterate over particles twice.
    reevaluateFollowership();
}

const FloatVector& Particle::getPressure() const {
    return pressure;
}

Direction Particle::getPressureDirection() const {
    const float x = pressure.getX();
    const float y = pressure.getY();
    const float x_abs = std::abs(x);
    const float y_abs = std::abs(y);
    if (x_abs > y_abs) {
        if (x > 0.0f) {
            return Direction::east();
        } else {
            return Direction::west();
        }
    } else {
        if (y > 0.0f) {
            return Direction::north();
        } else {
            return Direction::south();
        }
    }
}

void Particle::setTarget(const IntVector& target,
                         float target_pressure_per_second) {
    this->target = target;
    this->target_pressure_per_second = target_pressure_per_second;
}

void Particle::move(BlobStateKey, Direction direction) {
    assert(canMove() && "Particle was asked to move but can't.");
    const IntVector& vector = direction.vector();
    position += vector;
    // TODO Damp based on time step/distance traveled. Right now this just
    // subtracts the unit vector in movement direction.
    pressure -= static_cast<FloatVector>(vector);
}

void Particle::collideWith(Particle& forward_neighbor,
                           Direction collision_direction) {
    // TODO Parameterize how much of the pressure is passed on.
    switch (collision_direction) {
    case Direction::north():
    case Direction::south():
        forward_neighbor.pressure += FloatVector(0.0f, pressure.getY());
        pressure.setY(0.0f);
        break;
    case Direction::east():
    case Direction::west():
        forward_neighbor.pressure += FloatVector(pressure.getX(), 0.0f);
        pressure.setX(0.0f);
        break;
    }
    // Pass on our leaders to the particle we collided with, unless it's one of
    // the leaders, then just unfollow.
    for (Particle* leader: leaders) {
        leader->removeFollower(*this);
        if (leader != &forward_neighbor) {
            forward_neighbor.addLeader(*leader);
            leader->addFollower(forward_neighbor);
        }
    }
    leaders.clear();
}

void Particle::addFollowers(BlobStateKey,
                            const std::vector<Particle*> new_followers)
{
    // TODO Do I need this initial "boost", giving the new followers a part of
    // the pressure immediately? Or should I just let them get some on the next
    // updates?
    float divisor = static_cast<float>(followers.size() + 1);
    pressure /= divisor;
    this->followers.insert(new_followers.begin(), new_followers.end());
    addPressureToFollowers(new_followers, pressure.norm());
}

bool Particle::canMove() const {
    // TODO Unhardcode
    return pressure.getX() >= 1.0f || pressure.getY() >= 1.0f;
}

void Particle::addLeader(Particle& leader) {
    leaders.insert(&leader);
}

void Particle::addFollower(Particle& follower) {
    followers.insert(&follower);
}

void Particle::removeLeader(Particle& leader) {
    leaders.erase(&leader);
}

void Particle::removeFollower(Particle& follower) {
    followers.erase(&follower);
}

void Particle::reevaluateFollowership() {
    std::vector<Particle*> to_remove;
    std::vector<Particle*> to_switch;
    for (Particle* leader: leaders) {
        assert(leader != nullptr);
        if (pressure.dot(leader->pressure) < 0) {
            // The particles are trying to go in opposing directions and neither
            // should try to catch up to the other.
            to_remove.push_back(leader);
            continue;
        }
        IntVector to_leader = leader->position - position;
        if (to_leader.squaredNorm() <= 1) {
            // We're right next to the leader, stop following.
            to_remove.push_back(leader);
            continue;
        }
        FloatVector to_leader_float = static_cast<FloatVector>(to_leader);
        FloatVector pressures = pressure + leader->pressure;
        if (to_leader_float.dot(pressures) < 0) {
            // Both particles are going roughly the same way, but the follower
            // is ahead of the leader relative to the pressure direction. Switch
            // the leader-follower relationship.
            to_switch.push_back(leader);
        }
    }
    for (Particle* leader: to_remove) {
        leader->removeFollower(*this);
        removeLeader(*leader);
    }
    for (Particle* leader: to_switch) {
        leader->removeFollower(*this);
        removeLeader(*leader);
        addFollower(*leader);
        leader->addLeader(*this);
    }
}

}
