#include "../game/Vector.hpp"

#include <gtest/gtest.h>
#include <utility>
#include <cmath>
#include <algorithm>
#include <vector>

namespace wotmin2d {
namespace test {

TEST(Vector, defaultConstruction) {
    IntVector zero;
    EXPECT_EQ(0, zero.getX());
    EXPECT_EQ(0, zero.getY());
}

TEST(Vector, construction) {
    IntVector a(3, 5);
    EXPECT_EQ(3, a.getX());
    EXPECT_EQ(5, a.getY());
}

TEST(Vector, equality) {
    IntVector a(3, 5);
    IntVector b(3, 5);
    IntVector c(3, 6);
    IntVector d(4, 5);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
    EXPECT_TRUE(a != d);
}

TEST(Vector, copyConstruction) {
    IntVector a(3, 5);
    IntVector b(a);
    IntVector c(std::move(a));
    EXPECT_EQ(IntVector(3, 5), b);
    EXPECT_EQ(IntVector(3, 5), c);
}

TEST(Vector, assignment) {
    IntVector a(3, 5);
    IntVector b;
    b = a;
    EXPECT_EQ(IntVector(3, 5), b);
}

TEST(Vector, swap) {
    IntVector a(3, 5);
    IntVector b(7, 11);
    std::swap(a, b);
    EXPECT_EQ(IntVector(7, 11), a);
    EXPECT_EQ(IntVector(3, 5), b);
}

TEST(Vector, addition) {
    IntVector a(3, 5);
    IntVector b(7, 11);
    EXPECT_EQ(IntVector(10, 16), a + b);
    a += b;
    EXPECT_EQ(IntVector(10, 16), a);
}

TEST(Vector, subtraction) {
    IntVector a(3, 5);
    IntVector b(7, 11);
    EXPECT_EQ(IntVector(4, 6), b - a);
    b -= a;
    EXPECT_EQ(IntVector(4, 6), b);
}

TEST(Vector, scalarMultiplication) {
    IntVector a(3, 5);
    EXPECT_EQ(IntVector(21, 35), a * 7);
    a *= 7;
    EXPECT_EQ(IntVector(21, 35), a);
}

TEST(Vector, scalarDivision) {
    IntVector a(21, 35);
    EXPECT_EQ(IntVector(3, 5), a / 7);
    a /= 7;
    EXPECT_EQ(IntVector(3, 5), a);
}

TEST(Vector, accessAndMutation) {
    IntVector a(3, 5);
    EXPECT_EQ(3, a.getX());
    EXPECT_EQ(5, a.getY());
    a.setX(7);
    a.setY(11);
    EXPECT_EQ(7, a.getX());
    EXPECT_EQ(11, a.getY());
}

TEST(Vector, dot) {
    IntVector a(3, 5);
    IntVector b(7, 11);
    EXPECT_EQ(21 + 55, a.dot(b));
    EXPECT_EQ(21 + 55, b.dot(a));
}

TEST(Vector, squaredNorm) {
    IntVector a(3, 5);
    EXPECT_EQ(34, a.squaredNorm());
}

TEST(Vector, norm) {
    IntVector a(3, 5);
    float norm = std::sqrt(34);
    EXPECT_EQ(norm, a.norm());
}

TEST(Vector, intToFloatCast) {
    IntVector a(3, 5);
    EXPECT_EQ(FloatVector(3.0f, 5.0f), static_cast<FloatVector>(a));

    // x_i and y_i can't be exactly represented as floats
    int x_i = (1 << 26) - 1;
    int y_i = (1 << 26) - 2;
    float x_f = static_cast<float>(x_i);
    float y_f = static_cast<float>(y_i);
    IntVector b(x_i, y_i);
    FloatVector c = static_cast<FloatVector>(b);
    EXPECT_EQ(FloatVector(x_f, y_f), c);
}

TEST(Vector, intHashDoesntCollideForSmallPositiveValues) {
    // The limit for which this should work is actually 2^16 - 1, but testing
    // that uses 16GiB of memory (at least). This isn't ideal but at least fast.
    const int limit = 100;
    std::vector<IntVector::hash_t> hashes;
    hashes.reserve(limit * limit);
    IntVector::Hash hasher;
    for (int x = 0; x < limit; x++) {
        for (int y = 0; y < limit; y++) {
            IntVector::hash_t hash = hasher(IntVector(x, y));
            hashes.emplace_back(hash);
        }
    }
    // Make unique and check that everything is still there.
    std::sort(hashes.begin(), hashes.end());
    std::unique(hashes.begin(), hashes.end());
    EXPECT_EQ(limit * limit, hashes.size());
}

}
}
