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
#include <iterator>

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
    void setFollowers(BlobStateKey,
                      const std::vector<std::shared_ptr<Particle>>& followers,
                      Direction follower_direction);
    bool canMove();
    private:
    ParticlePositionState<Particle> position_state;
    ParticlePressureState pressure_state;
};

}

#endif
