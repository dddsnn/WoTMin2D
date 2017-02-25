#ifndef DIRECTION_HPP
#define DIRECTION_HPP

#include "Vector.hpp"

#include <cstdint>
#include <array>
#include <bitset>

namespace wotmin2d {

class Direction {
    public:
    using val_t = std::uint_fast8_t;
    constexpr static Direction north();
    constexpr static Direction south();
    constexpr static Direction west();
    constexpr static Direction east();
    constexpr static const std::array<Direction, 4>& all();
    constexpr const std::array<Direction, 3>& others() const;
    constexpr const std::bitset<4>& bitmask() const;
    constexpr operator val_t() const;
    constexpr Direction opposite() const;
    constexpr Direction left() const;
    constexpr IntVector vector() const;
    private:
    constexpr Direction(val_t value);
    // Changing these values without adjusting constants in direction_detail
    // will break this class. I know, I don't like it either, but can't figure
    // out a better way.
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

constexpr Direction::operator val_t() const {
    return value;
}

namespace direction_detail {
    // This namespace only contains constants relating to Direction. Can't put
    // them in the class itself, because constexpr members have to be defined
    // where they're declared, but at that point the class is still incomplete.
    constexpr static std::array<Direction, 4> all = { Direction::north(),
                                                      Direction::south(),
                                                      Direction::west(),
                                                      Direction::east()
                                                    };
    constexpr static std::array<Direction, 3> all_except_north
        = { Direction::south(), Direction::west(), Direction::east() };
    constexpr static std::array<Direction, 3> all_except_south
        = { Direction::north(), Direction::west(), Direction::east() };
    constexpr static std::array<Direction, 3> all_except_west
        = { Direction::north(), Direction::south(), Direction::east() };
    constexpr static std::array<Direction, 3> all_except_east
        = { Direction::north(), Direction::south(), Direction::west() };
    // The order of elements in the following arrays is dependent on the actual
    // values for the directions. This could get confusing if the values ever
    // change, but I can't think of a better solution.
    constexpr static std::array<std::array<Direction, 3>, 4> all_except
        = { all_except_north, all_except_west, all_except_south,
            all_except_east };
    constexpr static std::array<const IntVector, 4> vectors
        = { IntVector(0, 1), // north
            IntVector(-1, 0), // west
            IntVector(0, -1), // south
            IntVector(1, 0) // east
          };
    constexpr static std::array<const std::bitset<4>, 4> bitmasks
        = { std::bitset<4>(
                1 << static_cast<Direction::val_t>(Direction::north())
            ),
            std::bitset<4>(
                1 << static_cast<Direction::val_t>(Direction::west())
            ),
            std::bitset<4>(
                1 << static_cast<Direction::val_t>(Direction::south())
            ),
            std::bitset<4>(
                1 << static_cast<Direction::val_t>(Direction::east())
            )
          };
}

constexpr const std::array<Direction, 4>& Direction::all() {
    return direction_detail::all;
}

constexpr const std::array<Direction, 3>& Direction::others() const {
    return direction_detail::all_except[value];
}

constexpr const std::bitset<4>& Direction::bitmask() const {
    return direction_detail::bitmasks[value];
}

constexpr Direction Direction::opposite() const {
    return Direction((value + 2) % 4);
}

constexpr Direction Direction::left() const {
    return Direction((value + 1) % 4);
}

constexpr IntVector Direction::vector() const {
    return direction_detail::vectors[value];
}

}

#endif
