#include "ParticlePressureState.hpp"
#include "Particle.hpp"

namespace wotmin2d {

ParticlePressureState::ParticlePressureState() :
    target(0, 0),
    target_pressure(0.0f),
    pressure(0.0f, 0.0f) {
}

void ParticlePressureState::advance(const IntVector& current_position) {
    FloatVector to_target = static_cast<FloatVector>(target - current_position);
    FloatVector to_target_pressure = to_target
                                     * (target_pressure / to_target.norm());
    // TODO make dependent on time step
    pressure += to_target_pressure;
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

}
