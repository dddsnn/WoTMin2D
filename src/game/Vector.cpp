#include "Vector.hpp"

namespace wotmin2d {

Vector::Vector(int x, int y) :
    std::pair<int, int>(x, y) {
}

Vector Vector::operator-(const Vector& subtrahend) const {
    return Vector(first - subtrahend.first, second - subtrahend.second);
}

Vector Vector::operator+(const Vector& summand) const {
    return Vector(first + summand.first, second + summand.second);
}

bool Vector::operator==(const Vector& other) const {
    return first == other.first && second == other.second;
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

int Vector::Hash::operator()(const Vector& vector) const {
    // Use the lower half of each of the components as hash. Should be pretty
    // much unique, as long as values stay small and positive.
    const unsigned int bits_to_shift = (sizeof(int) * CHAR_BIT) / 2;
    int left = vector.getX() << bits_to_shift;
    int right = ((-1 >> bits_to_shift) & vector.getY());
    return left | right;
}

}
