#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <utility>

namespace wotmin2d {

class Vector : public std::pair<int, int> {
    public:
    Vector(int x, int y);
    Vector operator-(const Vector& subtrahend) const;
    int dot(const Vector& other) const;
    int getX() const;
    int getY() const;
    int& x();
    int& y();
};

}

#endif
