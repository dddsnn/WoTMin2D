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

template<typename T>
class WeakPtrHash {
    public:
    std::size_t operator()(const std::weak_ptr<T>& p) const {
        return std::hash<std::shared_ptr<T>>()(p.lock());
    }
};

// This equality compares equal even if the control blocks are different, i.e.
// the pointers were created from different shared pointers.
template<typename T>
class WeakPtrEqual {
    public:
    bool operator()(const std::weak_ptr<T>& first,
                    const std::weak_ptr<T>& second) const {
        return first.lock() == second.lock();
    }
};

class ParticlePressureState {
    public:
    ParticlePressureState();
    void advance(const IntVector& current_position);
    void setTarget(const IntVector& target, float target_pressure);
    void collideWith(ParticlePressureState& forward_neighbor_pressure_state);
    void updatePressureAfterMovement(const IntVector& vector);
    const FloatVector& getPressure() const;
    Direction getPressureDirection() const;
    void addFollowers(const std::vector<std::shared_ptr<Particle>>& followers);
    void addLeader(const std::shared_ptr<Particle>& leader,
                   const FloatVector& pressure);
    bool canMove() const;
    private:
    void dividePressure(FloatVector new_pressure);
    IntVector target;
    float target_pressure;
    FloatVector pressure;
    using ParticleSet = std::unordered_set<std::weak_ptr<Particle>,
                                           WeakPtrHash<Particle>,
                                           WeakPtrEqual<Particle>>;
    ParticleSet followers;
    ParticleSet leaders;
};

}

#endif
