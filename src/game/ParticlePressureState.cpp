#include "ParticlePressureState.hpp"
#include "Particle.hpp"

namespace wotmin2d {

ParticlePressureState::ParticlePressureState() :
    target(0, 0),
    target_pressure(0.0f),
    pressure(0.0f, 0.0f),
    followers(),
    leaders() {
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
    for (const std::weak_ptr<Particle>& follower: followers) {
        assert(follower.lock() != nullptr);
        follower.lock()->getPressureState({}).pressure += new_pressure;
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
    using std::swap;
    forward_neighbor_pressure_state.pressure += pressure;
    pressure = FloatVector(0.0f, 0.0f);
    // Stop following leaders after collision, unless they have no one else
    // following them.
    // TODO This means that, once a particle is followed, it will always have at
    // least one follower.
    ParticleSet new_leaders;
    for (const std::weak_ptr<Particle>& l: leaders) {
        std::shared_ptr<Particle> leader = l.lock();
        ParticlePressureState& leader_pps = leader->getPressureState({});
        // Make a new set containing all of the leader's followers.
        ParticleSet new_followers;
        std::weak_ptr<Particle> me;
        for (const std::weak_ptr<Particle>& follower: leader_pps.followers) {
            ParticlePressureState* follower_pps
                = &follower.lock()->getPressureState({});
            if (follower_pps == this) {
                me = follower;
                // Don't keep this particle. We may have to add it back in case
                // there are no other followers.
            } else if (follower.lock().get() != nullptr) {
                new_followers.insert(follower);
            }
        }
        assert(me.lock() != nullptr && "Leader didn't maintain following "
               "particle as a follower.");
        if (new_followers.empty()) {
            // The leader has no other followers than the current particle, keep
            // following it.
            new_followers.insert(me);
            new_leaders.insert(leader);
        }
        swap(leader_pps.followers, new_followers);
    }
    swap(leaders, new_leaders);
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
    const std::vector<std::shared_ptr<Particle>>& followers)
{
    // TODO Do I need this initial "boost", giving the new followers a part of
    // the pressure immediately? Or should I just let them get some on the next
    // updates?
    float divisor = static_cast<float>(followers.size() + 1);
    pressure /= divisor;
    this->followers.insert(followers.begin(), followers.end());
}

void ParticlePressureState::addLeader(const std::shared_ptr<Particle>& leader,
                                      const FloatVector& pressure) {
    leaders.insert(leader);
    this->pressure += pressure;
}

}
