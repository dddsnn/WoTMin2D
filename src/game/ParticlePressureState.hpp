#ifndef PARTICLEPRESSURESTATE_HPP
#define PARTICLEPRESSURESTATE_HPP

#include "Vector.hpp"
#include "Direction.hpp"

#include <memory>
#include <utility>
#include <algorithm>

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
    private:
    IntVector target;
    float target_pressure;
    FloatVector pressure;
};

}

#endif
