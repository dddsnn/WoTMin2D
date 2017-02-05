#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "Vector.hpp"

namespace wotmin2d {

class Particle {
    public:
    Particle(Vector position);
    const Vector& getPosition() const;
    private:
    Vector position_;
};

}

#endif
