#include "../game/Direction.hpp"
#include "../game/Vector.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <array>

namespace wotmin2d {
namespace test {

TEST(Direction, valuesArePairwiseDistinct) {
    EXPECT_NE(Direction::north(), Direction::west());
    EXPECT_NE(Direction::north(), Direction::south());
    EXPECT_NE(Direction::north(), Direction::east());
    EXPECT_NE(Direction::west(), Direction::south());
    EXPECT_NE(Direction::west(), Direction::east());
    EXPECT_NE(Direction::south(), Direction::east());
}

TEST(Direction, left) {
    EXPECT_EQ(Direction::west(), Direction::north().left());
    EXPECT_EQ(Direction::south(), Direction::west().left());
    EXPECT_EQ(Direction::east(), Direction::south().left());
    EXPECT_EQ(Direction::north(), Direction::east().left());
}

TEST(Direction, right) {
    EXPECT_EQ(Direction::east(), Direction::north().right());
    EXPECT_EQ(Direction::north(), Direction::west().right());
    EXPECT_EQ(Direction::west(), Direction::south().right());
    EXPECT_EQ(Direction::south(), Direction::east().right());
}

TEST(Direction, opposite) {
    EXPECT_EQ(Direction::south(), Direction::north().opposite());
    EXPECT_EQ(Direction::east(), Direction::west().opposite());
    EXPECT_EQ(Direction::north(), Direction::south().opposite());
    EXPECT_EQ(Direction::west(), Direction::east().opposite());
}

TEST(Direction, vector) {
    EXPECT_EQ(IntVector(0, 1), Direction::north().vector());
    EXPECT_EQ(IntVector(-1, 0), Direction::west().vector());
    EXPECT_EQ(IntVector(0, -1), Direction::south().vector());
    EXPECT_EQ(IntVector(1, 0), Direction::east().vector());
}

TEST(Direction, allContainsAllDirections) {
    const auto& all = Direction::all();
    EXPECT_EQ(1, std::count(all.begin(), all.end(), Direction::north()));
    EXPECT_EQ(1, std::count(all.begin(), all.end(), Direction::west()));
    EXPECT_EQ(1, std::count(all.begin(), all.end(), Direction::south()));
    EXPECT_EQ(1, std::count(all.begin(), all.end(), Direction::east()));
}

TEST(Direction, othersContainsOthersCounterClockwise) {
    for (Direction direction: Direction::all()) {
        Direction expected = direction.left();
        for (int i = 0; i < 3; i++) {
            EXPECT_EQ(expected, direction.others()[i]);
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
        EXPECT_EQ(1, std::count(values.begin(), values.end(), i));
    }
}

}
}
