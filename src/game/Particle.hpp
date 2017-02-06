#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "Vector.hpp"
#include "Direction.hpp"

namespace wotmin2d {

class Particle {
    public:
    Particle(Vector position);
    const Vector& getPosition() const;
    void move(Direction direction);
    private:
    Vector position_;
};

}

#endif
