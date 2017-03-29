#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "ParticlePositionState.hpp"
#include "ParticlePressureState.hpp"
#include "Vector.hpp"
#include "Direction.hpp"

#include <memory>
#include <array>
#include <utility>
#include <algorithm>
#include <initializer_list>

namespace wotmin2d {

class Particle {
    private:
    class MoveKey {
        friend class BlobState;
        private:
        MoveKey() {}
        MoveKey(const MoveKey&) = delete;
        MoveKey& operator=(const MoveKey&) = delete;
    };
    public:
    Particle(IntVector position);
    const IntVector& getPosition() const;
    std::shared_ptr<Particle> getNeighbor(Direction direction) const;
    void setNeighbor(MoveKey, Direction direction,
                     const std::shared_ptr<Particle>& neighbor);
    void move(MoveKey, Direction direction);
    bool hasPath(std::initializer_list<Direction> directions) const;
    const FloatVector& getPressure() const;
    Direction getPressureDirection() const;
    void advance();
    void setTarget(const IntVector& target, float target_pressure);
    void collideWith(Particle& forward_neighbor);
    private:
    ParticlePositionState<Particle> position_state;
    ParticlePressureState pressure_state;
};

}

#endif
