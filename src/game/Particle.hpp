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
    Particle(IntVector position);
    const IntVector& getPosition() const;
    void move(Direction direction);
    std::shared_ptr<Particle>& neighbor(Direction direction);
    private:
    IntVector position;
    std::array<std::shared_ptr<Particle>, 4> neighbors;
};

}

#endif
