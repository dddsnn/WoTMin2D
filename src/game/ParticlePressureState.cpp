#include "ParticlePressureState.hpp"
#include "Particle.hpp"

namespace wotmin2d {

ParticlePressureState::ParticlePressureState() :
    target(0, 0),
    target_pressure(0.0f),
    pressure(0.0f, 0.0f),
    followers(),
    leaders(),
    bubble_direction(Direction::north()) {
}

void ParticlePressureState::advance(const IntVector& current_position) {
    // TODO make dependent on time step
    FloatVector to_target = static_cast<FloatVector>(target - current_position);
    FloatVector to_target_pressure = to_target
                                     * (target_pressure / to_target.norm());
    dividePressure(std::move(to_target_pressure));
}

void ParticlePressureState::dividePressure(FloatVector new_pressure) {
    // TODO Clean out nulls from the follower set. This will be necessary when
    // particles can be removed by external means.
    // TODO Make it possible to give more of the pressure to the followers, so
    // they can follow more closely.
    float divisor = static_cast<float>(followers.size() + 1);
    new_pressure /= divisor;
    pressure += new_pressure;
    // TODO Instead of giving followers pressure in the same direction, maybe
    // they should get pressure in the direction towards their leader.
    for (ParticlePressureState* follower: followers) {
        assert(follower != nullptr);
        follower->pressure += new_pressure;
    }
}

void ParticlePressureState::setTarget(const IntVector& target,
                                      float target_pressure) {
    this->target = target;
    this->target_pressure = target_pressure;
}

void ParticlePressureState::updatePressureAfterMovement(const IntVector& vector)
{
    // TODO Damp based on time step/distance traveled. Right now this just
    // subtracts the unit vector in movement direction.
    pressure -= static_cast<FloatVector>(vector);
}

void ParticlePressureState::collideWith(
    ParticlePressureState& forward_neighbor_pressure_state)
{
    forward_neighbor_pressure_state.pressure += pressure;
    pressure = FloatVector(0.0f, 0.0f);
    // Pass on our leaders to the particle we collided with, unless it's one of
    // the leaders, then just unfollow.
    for (ParticlePressureState* leader: leaders) {
        leader->removeFollower(*this);
        if (leader != &forward_neighbor_pressure_state) {
            forward_neighbor_pressure_state.addLeader(*leader);
            leader->addFollower(forward_neighbor_pressure_state);
        }
    }
    leaders.clear();
}

const FloatVector& ParticlePressureState::getPressure() const {
    return pressure;
}

Direction ParticlePressureState::getPressureDirection() const {
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

void ParticlePressureState::addFollowers(
    const std::vector<ParticlePressureState*>& followers,
    Direction follower_direction)
{
    bubble_direction = follower_direction;
    // TODO Do I need this initial "boost", giving the new followers a part of
    // the pressure immediately? Or should I just let them get some on the next
    // updates?
    float divisor = static_cast<float>(followers.size() + 1);
    pressure /= divisor;
    this->followers.insert(followers.begin(), followers.end());
    for (ParticlePressureState* follower: followers) {
        follower->addLeader(*this);
        // Give the new follower a boost.
        follower->pressure += pressure;
    }
}

void ParticlePressureState::removeFollowers() {
    for (ParticlePressureState* follower: followers) {
        follower->removeLeader(*this);
    }
    followers.clear();
}

Direction ParticlePressureState::getBubbleDirection() const {
    return bubble_direction;
}

void ParticlePressureState::addLeader(ParticlePressureState& leader) {
    leaders.insert(&leader);
}

void ParticlePressureState::addFollower(ParticlePressureState& follower) {
    followers.insert(&follower);
}

void ParticlePressureState::removeLeader(ParticlePressureState& leader) {
    leaders.erase(&leader);
}

void ParticlePressureState::removeFollower(ParticlePressureState& follower) {
    followers.erase(&follower);
}

bool ParticlePressureState::canMove() const {
    // TODO Unhardcode
    bool has_pressure = pressure.getX() >= 1.0f || pressure.getY() >= 1.0f;
    bool has_bubble = !followers.empty();
    // We need pressure, and as long as we have followers we need to wait for
    // them (or another particle) to catch up and fill the bubble behind us.
    return has_pressure && !has_bubble;
}

}
