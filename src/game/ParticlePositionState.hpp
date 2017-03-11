#ifndef PARTICLEPOSITIONSTATE_HPP
#define PARTICLEPOSITIONSTATE_HPP

#include "Vector.hpp"
#include "Direction.hpp"

#include <memory>
#include <array>
#include <utility>
#include <algorithm>
#include <initializer_list>

namespace wotmin2d {

class Particle;

template<class Particle = Particle>
class ParticlePositionState {
    public:
    ParticlePositionState(IntVector position);
    const IntVector& getPosition() const;
    const std::shared_ptr<Particle>& getNeighbor(Direction direction) const;
    void setNeighbor(Direction direction,
                     const std::shared_ptr<Particle>& neighbor);
    bool hasPath(std::initializer_list<Direction> directions) const;
    void move(const IntVector& vector);
    private:
    IntVector position;
    std::array<std::shared_ptr<Particle>, 4> neighbors;
};

}

#include "ParticlePositionState.tpp"

#endif
