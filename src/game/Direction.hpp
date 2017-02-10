#ifndef DIRECTION_HPP
#define DIRECTION_HPP

#include "Vector.hpp"

#include <cstdint>
#include <array>

namespace wotmin2d {

class Direction {
    public:
    using val_t = std::uint_fast8_t;
    constexpr static Direction north();
    constexpr static Direction south();
    constexpr static Direction west();
    constexpr static Direction east();
    constexpr static const std::array<Direction, 4>& directions();
    constexpr operator val_t() const;
    constexpr Direction opposite() const;
    constexpr IntVector vector() const;
    private:
    constexpr Direction(val_t value);
    constexpr static val_t value_north = 0;
    constexpr static val_t value_west = 1;
    constexpr static val_t value_south = 2;
    constexpr static val_t value_east = 3;
    val_t value;
};

constexpr Direction::Direction(val_t value) :
    value(value) {
}

constexpr Direction Direction::north() {
    return Direction(value_north);
}

constexpr Direction Direction::south() {
    return Direction(value_south);
}

constexpr Direction Direction::west() {
    return Direction(value_west);
}

constexpr Direction Direction::east() {
    return Direction(value_east);
}

namespace direction_detail {
    // This namespace only contains a list of all possible directions. Can't put
    // it in the class itself, because constexpr members have to be defined
    // where they're declared, but at that point the class is still incomplete.
    constexpr static std::array<Direction, 4> directions = { Direction::north(),
                                                             Direction::south(),
                                                             Direction::west(),
                                                             Direction::east()
                                                           };
}

constexpr const std::array<Direction, 4>& Direction::directions() {
    return direction_detail::directions;
}

constexpr Direction::operator val_t() const {
    return value;
}

constexpr Direction Direction::opposite() const {
    return Direction((value + 2) % 4);
}

constexpr IntVector Direction::vector() const {
    // C++11 requires constexpr functions to be a single return statement. This
    // is not my fault.
    return value == value_north ? IntVector(0, 1) :
           value == value_south ? IntVector(0, -1) :
           value == value_west ? IntVector(-1, 0) :
           /* has to be east */ IntVector(1, 0);
}

}

#endif
