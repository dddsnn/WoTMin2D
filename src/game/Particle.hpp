#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "Vector.hpp"
#include "Direction.hpp"

#include <memory>
#include <stdexcept>

namespace wotmin2d {

class Particle {
    public:
    Particle(Vector position);
    const Vector& getPosition() const;
    void move(Direction direction);
    std::shared_ptr<Particle>& neighbor(Direction direction);
    private:
    Vector position_;
    std::shared_ptr<Particle> neighbor_north;
    std::shared_ptr<Particle> neighbor_south;
    std::shared_ptr<Particle> neighbor_west;
    std::shared_ptr<Particle> neighbor_east;
};

}

#endif
