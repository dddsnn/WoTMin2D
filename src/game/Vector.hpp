#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <utility>
#include <climits>
#include <cstdint>

namespace wotmin2d {

template<typename T>
class Vector {
    using hash_t = std::uint_fast32_t;
    public:
    Vector(T x, T y);
    Vector<T> operator-(const Vector<T>& subtrahend) const;
    Vector<T> operator+(const Vector<T>& summand) const;
    bool operator==(const Vector<T>& other) const;
    T dot(const Vector<T>& other) const;
    T getX() const;
    T getY() const;
    void setX(T x);
    void setY(T y);

    class Hash {
        public:
        hash_t operator()(const Vector<T>& vector) const;
    };
    private:
    T x;
    T y;
};

using IntVector = Vector<int>;

template<typename T>
Vector<T>::Vector(T x, T y) :
    x(x),
    y(y) {
}

template<typename T>
Vector<T> Vector<T>::operator-(const Vector<T>& subtrahend) const {
    return Vector<T>(x - subtrahend.x, y - subtrahend.y);
}

template<typename T>
Vector<T> Vector<T>::operator+(const Vector<T>& summand) const {
    return Vector(x + summand.x, y + summand.y);
}

template<typename T>
bool Vector<T>::operator==(const Vector& other) const {
    return x == other.x && y == other.y;
}

template<typename T>
T Vector<T>::dot(const Vector& other) const {
    return x * other.x + y * other.y;
}

template<typename T>
T Vector<T>::getX() const {
    return x;
}

template<typename T>
T Vector<T>::getY() const {
    return y;
}

template<typename T>
void Vector<T>::setX(T x) {
    this->x = x;
}

template<typename T>
void Vector<T>::setY(T y) {
    this->y = y;
}

}

#endif
