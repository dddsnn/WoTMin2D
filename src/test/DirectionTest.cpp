#include "../game/Direction.hpp"
#include "../game/Vector.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <array>

namespace wotmin2d {
namespace test {

TEST(Direction, valuesArePairwiseDistinct) {
    ASSERT_NE(Direction::north(), Direction::west());
    ASSERT_NE(Direction::north(), Direction::south());
    ASSERT_NE(Direction::north(), Direction::east());
    ASSERT_NE(Direction::west(), Direction::south());
    ASSERT_NE(Direction::west(), Direction::east());
    ASSERT_NE(Direction::south(), Direction::east());
}

TEST(Direction, left) {
    ASSERT_EQ(Direction::west(), Direction::north().left());
    ASSERT_EQ(Direction::south(), Direction::west().left());
    ASSERT_EQ(Direction::east(), Direction::south().left());
    ASSERT_EQ(Direction::north(), Direction::east().left());
}

TEST(Direction, right) {
    ASSERT_EQ(Direction::east(), Direction::north().right());
    ASSERT_EQ(Direction::north(), Direction::west().right());
    ASSERT_EQ(Direction::west(), Direction::south().right());
    ASSERT_EQ(Direction::south(), Direction::east().right());
}

TEST(Direction, opposite) {
    ASSERT_EQ(Direction::south(), Direction::north().opposite());
    ASSERT_EQ(Direction::east(), Direction::west().opposite());
    ASSERT_EQ(Direction::north(), Direction::south().opposite());
    ASSERT_EQ(Direction::west(), Direction::east().opposite());
}

TEST(Direction, vector) {
    ASSERT_EQ(IntVector(0, 1), Direction::north().vector());
    ASSERT_EQ(IntVector(-1, 0), Direction::west().vector());
    ASSERT_EQ(IntVector(0, -1), Direction::south().vector());
    ASSERT_EQ(IntVector(1, 0), Direction::east().vector());
}

TEST(Direction, allContainsAllDirections) {
    const auto& all = Direction::all();
    ASSERT_EQ(1, std::count(all.begin(), all.end(), Direction::north()));
    ASSERT_EQ(1, std::count(all.begin(), all.end(), Direction::west()));
    ASSERT_EQ(1, std::count(all.begin(), all.end(), Direction::south()));
    ASSERT_EQ(1, std::count(all.begin(), all.end(), Direction::east()));
}

TEST(Direction, othersContainsOthersCounterClockwise) {
    for (Direction direction: Direction::all()) {
        Direction expected = direction.left();
        for (int i = 0; i < 3; i++) {
            ASSERT_EQ(expected, direction.others()[i]);
            expected = expected.left();
        }
    }
}

TEST(Direction, valueCanBeUsedAsArrayIndex) {
    const auto& all = Direction::all();
    std::array<Direction::val_t, 3> values;
    std::transform(all.begin(), all.end(), values.begin(), [](Direction d) {
        return static_cast<Direction::val_t>(d);
    });
    for (int i = 0; i < 3; i++) {
        ASSERT_EQ(1, std::count(values.begin(), values.end(), i));
    }
}

}
}
