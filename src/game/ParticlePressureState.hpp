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
    using Movement = std::pair<Direction, bool>;
    ParticlePressureState();
    Movement getMovement() const;
    void advance(const IntVector& current_position);
    void setTarget(const IntVector& target, float target_pressure);
    void collideWith(ParticlePressureState& forward_neighbor_pressure_state);
    void updatePressureAfterMovement(const IntVector& vector);
    private:
    IntVector target;
    float target_pressure;
    FloatVector pressure;
    Direction getPressureDirection() const;
};

}

#endif
