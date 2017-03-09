#include "mock/MockBlobState.hpp"
#include "../game/BlobState.hpp"
#include "../game/Blob.hpp"
#include "../game/Vector.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <initializer_list>

namespace wotmin2d {
namespace test {

using mock::MockBlobState;

using ::testing::AtLeast;
using ::testing::_;

using ParticlePtr = std::shared_ptr<Particle>;

class BlobTest : public ::testing::Test {
    protected:
    BlobTest() :
        state(std::make_shared<MockBlobState>()),
        inSouthWestCorner(IntVector(0, 0)),
        onSouthBorder(IntVector(10, 0)),
        inside(IntVector(15, 5)),
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
    }
    static std::vector<ParticlePtr>
        makeParticles(std::initializer_list<std::vector<IntVector>> vectors,
                      std::initializer_list<IntVector> singles) {
        // Use BlobState to create vectors of particles that have their neighbor
        // pointers properly set.
        auto b = std::unique_ptr<BlobState>(new BlobState());
        for (const auto& v: vectors) {
            for (const auto& p: v) {
                b->addParticle(p);
            }
        }
        for (const auto& p: singles) {
            b->addParticle(p);
        }
        return b->getParticles();
    }
    std::shared_ptr<MockBlobState> state;
    Particle inSouthWestCorner;
    Particle onSouthBorder;
    Particle inside;
    std::vector<IntVector> lineA;
    std::vector<IntVector> lineB;
    std::vector<IntVector> block;
    std::vector<IntVector> loop;
    static const unsigned int width = 21;
    static const unsigned int height = 41;
};

TEST_F(BlobTest, normalConstructorDoesntAddParticles) {
    EXPECT_CALL(*state, addParticle(_))
        .Times(0);
    Blob<MockBlobState> blob(width, height, state);
}

// TODO Test with the exact number and position of particles.
TEST_F(BlobTest, circleConstructorAddsParticles) {
    EXPECT_CALL(*state, addParticle(_))
        .Times(AtLeast(1));
    Blob<MockBlobState> blob(IntVector(10, 20), 3.0f, width, height, state);
}

TEST_F(BlobTest, advancesEachParticleOnce) {
// TODO Mock Particle to test each particle is advanced once.
}

}
}
