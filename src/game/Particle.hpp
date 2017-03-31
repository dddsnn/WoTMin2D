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
    class BlobStateKey {
        friend class BlobState;
        private:
        BlobStateKey() {}
        BlobStateKey(const BlobStateKey&) = delete;
        BlobStateKey& operator=(const BlobStateKey&) = delete;
    };
    class PressureStateKey {
        friend class ParticlePressureState;
        private:
        PressureStateKey() {}
        PressureStateKey(const PressureStateKey&) = delete;
        PressureStateKey& operator=(const PressureStateKey&) = delete;
    };
    public:
    Particle(IntVector position);
    const IntVector& getPosition() const;
    std::shared_ptr<Particle> getNeighbor(Direction direction) const;
    void setNeighbor(BlobStateKey, Direction direction,
                     const std::shared_ptr<Particle>& neighbor);
    bool hasNeighbor() const;
    void move(BlobStateKey, Direction direction);
    bool hasPath(std::initializer_list<Direction> directions) const;
    const FloatVector& getPressure() const;
    Direction getPressureDirection() const;
    void advance();
    void setTarget(const IntVector& target, float target_pressure);
    void collideWith(Particle& forward_neighbor);
    void addFollowers(BlobStateKey,
                      const std::vector<std::shared_ptr<Particle>>& followers);
    void addLeader(BlobStateKey, const std::shared_ptr<Particle>& leader,
                   const FloatVector& pressure);
    ParticlePressureState& getPressureState(PressureStateKey);
    private:
    ParticlePositionState<Particle> position_state;
    ParticlePressureState pressure_state;
};

}

#endif
