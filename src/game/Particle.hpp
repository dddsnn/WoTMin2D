#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "Vector.hpp"
#include "Direction.hpp"

#include <memory>
#include <stdexcept>
#include <array>
#include <cmath>
#include <utility>
#include <algorithm>

namespace wotmin2d {

class Particle {
    public:
    using Movement = std::pair<Direction, bool>;
    Particle(IntVector position);
    const IntVector& getPosition() const;
    std::shared_ptr<Particle>& neighbor(Direction direction);
    Movement getMovement() const;
    void advance();
    void setTarget(const IntVector& target, float target_pressure);
    void move(Direction direction);
    void collideWith(Particle& forward_neighbor);
    private:
    IntVector position;
    IntVector target;
    float target_pressure;
    FloatVector pressure;
    std::array<std::shared_ptr<Particle>, 4> neighbors;
    Direction getPressureDirection() const;
};

}

#endif
