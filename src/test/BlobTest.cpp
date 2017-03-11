#include "mock/MockBlobState.hpp"
#include "mock/MockParticle.hpp"
#include "../game/BlobState.hpp"
#include "../game/Blob.hpp"
#include "../game/Vector.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <algorithm>
#include <iterator>

namespace wotmin2d {
namespace test {

using mock::MockBlobState;
using mock::MockParticle;

using ::testing::AtLeast;
using ::testing::_;
using ::testing::ReturnRef;

using ParticlePtr = std::shared_ptr<MockParticle>;

class BlobTest : public ::testing::Test {
    protected:
    BlobTest() :
        state(std::make_shared<MockBlobState>()),
        particles(),
        inSouthWestCorner(0, 0),
        onSouthBorder(10, 0),
        inside(15, 5),
        lineA(),
        lineB(),
        block(),
        loop() {
        // lineA: from (3, 2) to (3, 10)
        for (int i = 2; i <= 10; i++) {
            lineA.emplace_back(IntVector(3, i));
        }
        // lineB: from (4, 10) to (10, 10)
        for (int i = 4; i <= 10; i++) {
            lineB.emplace_back(IntVector(i, 10));
        }
        // block: square from (4, 1) to (7, 4)
        for (int i = 4; i <= 7; i++) {
            for (int j = 1; j <= 4; j++) {
                block.emplace_back(IntVector(i, j));
            }
        }
        // loop: square from (10, 11) to (12, 13) with middle particle missing
        for (int i = 10; i <= 12; i++) {
            for (int j = 11; j <= 13; j++) {
                if (i == 11 && j == 12) {
                    continue;
                }
                block.emplace_back(IntVector(i, j));
            }
        }
        // Return the fixtures particles by default.
        ON_CALL(*state, getParticles()).WillByDefault(ReturnRef(particles));
    }
    virtual ~BlobTest() {
        clearParticles();
    }
    virtual void clearParticles() {
        // Unset neighbor pointers so no memory is leaked through cyclic
        // dependencies.
        for (auto& p: particles) {
            for (auto d: Direction::all()) {
                p->setNeighbor({}, d, nullptr);
            }
        }
        particles.clear();
    }
    virtual void
    makeParticles(std::initializer_list<std::vector<IntVector>> vectors,
                  std::initializer_list<IntVector> singles)
    {
        clearParticles();
        std::unordered_map<IntVector, ParticlePtr, IntVector::Hash> map;
        for (const auto& v: vectors) {
            for (const auto& p: v) {
                map.emplace(p, std::make_shared<MockParticle>(p));
            }
        }
        for (const auto& p: singles) {
            map.emplace(p, std::make_shared<MockParticle>(p));
        }
        auto getSecond = [] (std::pair<const IntVector, ParticlePtr>& v) {
                           return v.second;
                         };
        std::transform(map.begin(), map.end(), std::back_inserter(particles),
                       getSecond);
        // Set neighbors.
        for (const auto& p: particles) {
            for (auto d: Direction::all()) {
                auto i = map.find(p->getPosition() + d.vector());
                if (i != map.end()) {
                    p->setNeighbor(MockParticle::MoveKey(), d, i->second);
                }
            }
        }
    }
    std::shared_ptr<MockBlobState> state;
    std::vector<ParticlePtr> particles;
    IntVector inSouthWestCorner;
    IntVector onSouthBorder;
    IntVector inside;
    std::vector<IntVector> lineA;
    std::vector<IntVector> lineB;
    std::vector<IntVector> block;
    std::vector<IntVector> loop;
    static const unsigned int width = 21;
    static const unsigned int height = 41;
};

TEST_F(BlobTest, normalConstructorDoesntAddParticles) {
    EXPECT_CALL(*state, addParticle(_)).Times(0);
    Blob<MockBlobState> blob(width, height, state);
}

// TODO Test with the exact number and position of particles.
TEST_F(BlobTest, circleConstructorAddsParticles) {
    EXPECT_CALL(*state, addParticle(_)).Times(AtLeast(1));
    Blob<MockBlobState> blob(IntVector(10, 20), 3.0f, width, height, state);
}

TEST_F(BlobTest, advancesEachParticleOnce) {
    makeParticles({ lineA, lineB, block, loop },
                  { inSouthWestCorner, onSouthBorder, inside});
    for (const auto& p: particles) {
        EXPECT_CALL(*p, advance()).Times(1);
    }
    Blob<MockBlobState, MockParticle> blob(width, height, state);
    blob.advance();
}

}
}
