#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(IntVector position) :
    position(position),
    neighbors(),
    target(0, 0),
    target_pressure_per_second(0.0f),
    pressure(0.0f, 0.0f),
    followers(),
    leaders(),
    // TODO unhardcode
    health(100) {}

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

void Particle::advance(BlobStateKey, std::chrono::milliseconds time_delta) {
    // How much target pressure do we need to apply?
    std::chrono::duration<float, std::ratio<1>> second_fraction = time_delta;
    float target_pressure = second_fraction.count()
                            * target_pressure_per_second;
    // How much of it does each particle get?
    static_assert(Config::target_pressure_share <= 1.0f
                  && Config::target_pressure_share >= 0.0f,
                  "A share outside of [0, 1] may artificially inflate or "
                  "deflate the pressure.");
    float divisor = static_cast<float>(followers.size())
                    + Config::target_pressure_share;
    if (divisor < 1.0f) {
        // There are no followers, take all the pressure.
        divisor = 1.0f;
    }
    float pressure_part = target_pressure / divisor;
    FloatVector to_target = static_cast<FloatVector>(target - position);
    if (to_target != FloatVector(0.0f, 0.0f)) {
        // Possibly give this particle a smaller share so the followers can have
        // more.
        float this_part = pressure_part * Config::target_pressure_share;
        FloatVector to_target_pressure = to_target
                                         * (this_part / to_target.norm());
        pressure += to_target_pressure;
    }
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
    assert(direction == getPressureDirection() && "Particle was asked to move "
           "in a different direction than the pressure.");
    const IntVector& vector = direction.vector();
    position += vector;
    pressure -= static_cast<FloatVector>(vector);
}

void Particle::collideWith(BlobStateKey, Particle& forward_neighbor,
                           Direction collision_direction) {
    switch (collision_direction) {
    case Direction::north():
    case Direction::south():
        forward_neighbor.pressure
            += FloatVector(0.0f, pressure.getY() * Config::collision_pass_on);
        pressure.setY(pressure.getY() * (1.0f - Config::collision_pass_on));
        break;
    case Direction::east():
    case Direction::west():
        forward_neighbor.pressure
            += FloatVector(pressure.getX() * Config::collision_pass_on, 0.0f);
        pressure.setX(pressure.getX() * (1.0f - Config::collision_pass_on));
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

void Particle::killPressureInDirection(BlobStateKey, Direction direction) {
    switch (direction) {
    case Direction::north():
    case Direction::south():
        pressure.setY(0.0f);
        break;
    case Direction::west():
    case Direction::east():
        pressure.setX(0.0f);
        break;
    }
}

bool Particle::canMove() const {
    // TODO Using a constant here only works as long as the movements a particle
    // is asked to make have fixed length (of 1). If that ever changes, this
    // needs to be parameterized on the length of the movement, i.e. "can this
    // particle move 6 to the east?".
    return std::abs(pressure.getX()) >= Config::min_directed_movement_pressure
           || std::abs(pressure.getY())
              >= Config::min_directed_movement_pressure;
}

void Particle::addFollowers(BlobStateKey,
                            const std::vector<Particle*>& new_followers)
{
    static_assert(Config::boost_fraction >= 0.0f
                  && Config::boost_fraction <= 1.0f,
                  "A boost fraction outside [0, 1] will artificially inflate or"
                  " deflate the pressure.");
    this->followers.insert(new_followers.begin(), new_followers.end());
    for (Particle* new_follower: new_followers) {
        new_follower->addLeader(*this);
    }
    float pressure_magnitude = pressure.norm();
    float boost_magnitude = Config::boost_fraction * pressure_magnitude;
    float divisor = static_cast<float>(followers.size());
    pressure *= (1.0f - Config::boost_fraction);
    addPressureToFollowers(new_followers, boost_magnitude / divisor);
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

void Particle::removeLeader(BlobStateKey, Particle& leader) {
    removeLeader(leader);
}

void Particle::removeFollower(BlobStateKey, Particle& follower) {
    removeFollower(follower);
}

std::unordered_set<Particle*>& Particle::getFollowers(BlobStateKey) {
    return followers;
}

std::unordered_set<Particle*>& Particle::getLeaders(BlobStateKey) {
    return leaders;
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

unsigned int Particle::getHealth() const {
    return health;
}

void Particle::damage(BlobStateKey, unsigned int amount) {
    health = amount < health ? health - amount : 0;
}

}
