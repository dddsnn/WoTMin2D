#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <utility>
#include <climits>

namespace wotmin2d {

class Vector : public std::pair<int, int> {
    public:
    Vector(int x, int y);
    Vector operator-(const Vector& subtrahend) const;
    Vector operator+(const Vector& summand) const;
    bool operator==(const Vector& other) const;
    int dot(const Vector& other) const;
    int getX() const;
    int getY() const;
    int& x();
    int& y();

    class Hash {
        public:
        int operator()(const Vector& vector) const;
    };
};

}

#endif
