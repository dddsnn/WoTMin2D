#ifndef PARTICLE_HPP
#define PARTICLE_HPP

namespace wotmin2d {

class Particle {
    public:
    Particle(unsigned int x, unsigned int y);
    unsigned int getX() const;
    unsigned int getY() const;
    void setPosition(unsigned int x, unsigned int y);
    private:
    unsigned int x;
    unsigned int y;
};

}

#endif
