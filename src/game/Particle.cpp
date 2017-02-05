#include "Particle.hpp"

namespace wotmin2d {

Particle::Particle(unsigned int x, unsigned int y) :
    x(x),
    y(y) {
}

unsigned int Particle::getX() const {
    return x;
}

unsigned int Particle::getY() const {
    return y;
}

void Particle::setPosition(unsigned int x, unsigned int y) {
    this->x = x;
    this->y = y;
}

}
