#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <utility>
#include <climits>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace wotmin2d {

template<typename T>
class Vector {
    public:
    using hash_t = std::uint_fast32_t;
    Vector();
    constexpr Vector(const Vector<T>& other);
    Vector(Vector<T>&& other);
    constexpr Vector(const T& x, const T& y);
    Vector<T>& operator=(Vector<T> other);
    template<typename U>
    friend void swap(Vector<U>& first, Vector<U>& second) noexcept;
    Vector<T> operator+(const Vector<T>& summand) const;
    Vector<T> operator-(const Vector<T>& subtrahend) const;
    Vector<T> operator*(const T& scalar) const;
    Vector<T> operator/(const T& scalar) const;
    Vector<T>& operator+=(const Vector<T>& summand);
    Vector<T>& operator-=(const Vector<T>& subtrahend);
    Vector<T>& operator*=(const T& scalar);
    Vector<T>& operator/=(const T& scalar);
    bool operator==(const Vector<T>& other) const;
    bool operator!=(const Vector<T>& other) const;
    template<typename U>
    explicit operator Vector<U>() const;
    T dot(const Vector<T>& other) const;
    float norm() const;
    T squaredNorm() const;
    const T& getX() const;
    const T& getY() const;
    void setX(const T& x);
    void setY(const T& y);

    class Hash {
        public:
        hash_t operator()(const Vector<T>& vector) const;
    };
    private:
    T x;
    T y;
};

using IntVector = Vector<int>;
using FloatVector = Vector<float>;

template<typename T>
Vector<T>::Vector() :
    x(),
    y() {
}

template<typename T>
constexpr Vector<T>::Vector(const Vector<T>& other) :
    x(other.x),
    y(other.y) {
}

template<typename T>
Vector<T>::Vector(Vector<T>&& other) :
    x(),
    y() {
    swap(*this, other);
}

template<typename T>
constexpr Vector<T>::Vector(const T& x, const T& y) :
    x(x),
    y(y) {
}

template<typename T>
Vector<T>& Vector<T>::operator=(Vector<T> other) {
    swap(*this, other);
    return *this;
}

template<typename T>
void swap(Vector<T>& first, Vector<T>& second) noexcept {
    using std::swap;
    swap(first.x, second.x);
    swap(first.y, second.y);
}

template<typename T>
Vector<T> Vector<T>::operator+(const Vector<T>& summand) const {
    return Vector(x + summand.x, y + summand.y);
}

template<typename T>
Vector<T> Vector<T>::operator-(const Vector<T>& subtrahend) const {
    return Vector<T>(x - subtrahend.x, y - subtrahend.y);
}

template<typename T>
Vector<T> Vector<T>::operator*(const T& scalar) const {
    return Vector(x * scalar, y * scalar);
}

template<typename T>
Vector<T> Vector<T>::operator/(const T& scalar) const {
    return Vector(x / scalar, y / scalar);
}

template<typename T>
Vector<T>& Vector<T>::operator+=(const Vector<T>& summand) {
    x += summand.x;
    y += summand.y;
    return *this;
}

template<typename T>
Vector<T>& Vector<T>::operator-=(const Vector<T>& subtrahend) {
    x -= subtrahend.x;
    y -= subtrahend.y;
    return *this;
}

template<typename T>
Vector<T>& Vector<T>::operator*=(const T& scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

template<typename T>
Vector<T>& Vector<T>::operator/=(const T& scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

template<typename T>
bool Vector<T>::operator==(const Vector& other) const {
    return x == other.x && y == other.y;
}

template<typename T>
bool Vector<T>::operator!=(const Vector& other) const {
    return !(*this == other);
}

template<typename T>
T Vector<T>::dot(const Vector& other) const {
    return x * other.x + y * other.y;
}

template<typename T>
float Vector<T>::norm() const {
    return std::sqrt(static_cast<float>(squaredNorm()));
}

template<typename T>
T Vector<T>::squaredNorm() const {
    return this->dot(*this);
}

template<typename T>
const T& Vector<T>::getX() const {
    return x;
}

template<typename T>
const T& Vector<T>::getY() const {
    return y;
}

template<typename T>
void Vector<T>::setX(const T& x) {
    this->x = x;
}

template<typename T>
void Vector<T>::setY(const T& y) {
    this->y = y;
}

}

#endif
