#include "Vector.hpp"

namespace wotmin2d {

template<>
Vector<int>::hash_t Vector<int>::Hash::operator()(const Vector<int>& vector)
        const {
    // Use the lower half of each of the components as hash. Should be pretty
    // much unique, as long as values stay small and positive.
    const hash_t x = static_cast<hash_t>(vector.getX());
    const hash_t y = static_cast<hash_t>(vector.getY());
    const unsigned int bits_to_shift = (sizeof(hash_t) * CHAR_BIT) / 2;
    const hash_t left = x << bits_to_shift;
    const hash_t right_ones = static_cast<hash_t>(-1) >> bits_to_shift;
    const hash_t right = right_ones & y;
    return left | right;
}

template<> template<>
Vector<int>::operator Vector<float>() const {
    return Vector<float>(static_cast<float>(x), static_cast<float>(y));
}

}
