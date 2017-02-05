#include "Vector.hpp"

namespace wotmin2d {

Vector::Vector(int x, int y) :
    std::pair<int, int>(x, y) {
}

Vector Vector::operator-(const Vector& subtrahend) const {
    return Vector(first - subtrahend.first, second - subtrahend.second);
}

int Vector::dot(const Vector& other) const {
    return first * other.first + second * other.second;
}

int Vector::getX() const {
    return first;
}

int Vector::getY() const {
    return second;
}

int& Vector::x() {
    return first;
}

int& Vector::y() {
    return second;
}

}
