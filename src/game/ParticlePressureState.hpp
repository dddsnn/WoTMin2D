#ifndef PARTICLEPRESSURESTATE_HPP
#define PARTICLEPRESSURESTATE_HPP

#include "Vector.hpp"
#include "Direction.hpp"

#include <memory>
#include <utility>
#include <algorithm>
#include <unordered_set>
#include <cstddef>
#include <functional>
#include <cassert>

namespace wotmin2d {

class Particle;

class ParticlePressureState {
    public:
    ParticlePressureState();
    void advance(const IntVector& current_position);
    void setTarget(const IntVector& target, float target_pressure);
    void collideWith(ParticlePressureState& forward_neighbor_pressure_state);
    void updatePressureAfterMovement(const IntVector& vector);
    const FloatVector& getPressure() const;
    Direction getPressureDirection() const;
    void addFollowers(const std::vector<ParticlePressureState*>& followers,
                      Direction follower_direction);
    void removeFollowers();
    Direction getBubbleDirection() const;
    bool canMove() const;
    private:
    void dividePressure(FloatVector new_pressure);
    void addLeader(ParticlePressureState& leader);
    void addFollower(ParticlePressureState& follower);
    void removeLeader(ParticlePressureState& leader);
    void removeFollower(ParticlePressureState& follower);
    IntVector target;
    float target_pressure;
    FloatVector pressure;
    std::unordered_set<ParticlePressureState*> followers;
    std::unordered_set<ParticlePressureState*> leaders;
    Direction bubble_direction;
};

}

#endif
