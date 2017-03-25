#include "mock/MockParticle.hpp"
#include "TestData.hpp"

namespace wotmin2d {
namespace test {

using mock::NiceMockParticle;

using ParticlePtr = std::shared_ptr<NiceMockParticle>;

class ParticlePositionStateTest : public ::testing::Test {
    protected:
    ParticlePositionStateTest() :
        td() {
    }
    TestData td;
};

TEST_F(ParticlePositionStateTest, recognizesPathToItself) {
    td.makeParticles({}, { td.inside });
    EXPECT_TRUE(td.particles.front()->hasPath({}));
}

TEST_F(ParticlePositionStateTest, recognizesPathsThatExist) {
    td.makeParticles({ td.lineA, td.lineB }, {});
    const ParticlePtr& beforeCorner = td.particle_map[IntVector(3, 9)];
    EXPECT_TRUE(beforeCorner->hasPath({ Direction::north() }));
    EXPECT_TRUE(beforeCorner->hasPath({ Direction::north(),
                                        Direction::east() }));
}

TEST_F(ParticlePositionStateTest, recognizesPathsOnLoop) {
    td.makeParticles({ td.loop }, {});
    const ParticlePtr& onLoop = td.particle_map[IntVector(10, 12)];
    EXPECT_TRUE(onLoop->hasPath({ Direction::north(),
                                  Direction::east(),
                                  Direction::east(),
                                  Direction::south(),
                                  Direction::south(),
                                  Direction::west(),
                                  Direction::west(),
                                  Direction::north() }));
}

TEST_F(ParticlePositionStateTest, recognizesThatPathsDontExist) {
    td.makeParticles({ td.loop }, {});
    const ParticlePtr& onLoop = td.particle_map[IntVector(10, 12)];
    EXPECT_FALSE(onLoop->hasPath({ Direction::east() }));
    EXPECT_FALSE(onLoop->hasPath({ Direction::west() }));
}

TEST_F(ParticlePositionStateTest, movesByZero) {
    ParticlePositionState<NiceMockParticle> pps(td.inside);
    IntVector before = pps.getPosition();
    pps.move(IntVector(0, 0));
    EXPECT_EQ(before, pps.getPosition());
}

TEST_F(ParticlePositionStateTest, movesByOne) {
    ParticlePositionState<NiceMockParticle> pps(td.inside);
    IntVector before = pps.getPosition();
    IntVector moveBy(0, 1);
    pps.move(moveBy);
    EXPECT_EQ(before + moveBy, pps.getPosition());
}

TEST_F(ParticlePositionStateTest, movesByMore) {
    ParticlePositionState<NiceMockParticle> pps(td.inside);
    IntVector before = pps.getPosition();
    IntVector moveBy(-7, 12);
    pps.move(moveBy);
    EXPECT_EQ(before + moveBy, pps.getPosition());
}

}
}
