#include "../game/ParticlePressureState.hpp"
#include "../game/Vector.hpp"
#include "../game/Direction.hpp"

#include <gtest/gtest.h>

namespace wotmin2d {
namespace test {

class ParticlePressureStateTest : public ::testing::Test {
    protected:
    ParticlePressureStateTest() :
        start_position(5, 5) {}
    IntVector start_position;
};

TEST_F(ParticlePressureStateTest, doesntWantToMoveInitially) {
    ParticlePressureState pps;
    EXPECT_EQ(FloatVector(0.0f, 0.0f), pps.getPressure());
}

TEST_F(ParticlePressureStateTest, doesntWantToMoveWithTargetBeforeAdvancing) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(5, 0), 1.0f);
    EXPECT_EQ(FloatVector(0.0f, 0.0f), pps.getPressure());
}

TEST_F(ParticlePressureStateTest, wantsToMoveAfterAdvancing) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(5, 0), 1.0f);
    pps.advance(start_position);
    EXPECT_EQ(Direction::east(), pps.getPressureDirection());
    EXPECT_GT(pps.getPressure().dot(FloatVector(1.0f, 0.0f)), 0.0f);
}

TEST_F(ParticlePressureStateTest, canChangeDirection) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(5, 0), 1.0f);
    pps.advance(start_position);
    pps.updatePressureAfterMovement(Direction::east().vector());
    pps.setTarget(start_position - IntVector(5, 0), 1.0f);
    pps.advance(start_position + Direction::east().vector());
    EXPECT_EQ(Direction::west(), pps.getPressureDirection());
    EXPECT_GT(pps.getPressure().dot(FloatVector(-1.0f, 0.0f)), 0.0f);
}

TEST_F(ParticlePressureStateTest, canMoveInNonStraightLines) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(2, 1), 1.0f);
    pps.advance(start_position);
    EXPECT_EQ(Direction::east(), pps.getPressureDirection());
    EXPECT_GT(pps.getPressure().dot(FloatVector(1.0f, 0.0f)), 0.0f);
    pps.updatePressureAfterMovement(Direction::east().vector());
    pps.advance(start_position + Direction::east().vector());
    EXPECT_EQ(Direction::north(), pps.getPressureDirection());
    EXPECT_GT(pps.getPressure().dot(FloatVector(0.0f, 1.0f)), 0.0f);
}

TEST_F(ParticlePressureStateTest,
       continuesMovingWithoutTargetWithRemainingPressureUntilItRunsOut) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(1, 0), 2.0f);
    pps.advance(start_position);
    pps.updatePressureAfterMovement(Direction::east().vector());
    pps.setTarget(start_position, 0.0f);
    pps.advance(start_position + Direction::east().vector());
    EXPECT_EQ(Direction::east(), pps.getPressureDirection());
    EXPECT_GT(pps.getPressure().dot(FloatVector(1.0f, 0.0f)), 0.0f);
    pps.updatePressureAfterMovement(Direction::east().vector());
    pps.advance(start_position + Direction::east().vector() * 2);
    EXPECT_EQ(FloatVector(0.0f, 0.0f), pps.getPressure());
}

TEST_F(ParticlePressureStateTest, passesOnEntirePressureOnCollision) {
    ParticlePressureState pps;
    ParticlePressureState neighbor;
    pps.setTarget(start_position + IntVector(5, 0), 1.0f);
    pps.advance(start_position);
    neighbor.advance(start_position + IntVector(1, 0));
    FloatVector pps_pressure_before = pps.getPressure();
    FloatVector neighbor_pressure_before = neighbor.getPressure();
    pps.collideWith(neighbor);
    EXPECT_EQ(FloatVector(0.0f, 0.0f), pps.getPressure());
    EXPECT_EQ(pps_pressure_before + neighbor_pressure_before,
              neighbor.getPressure());
    EXPECT_EQ(Direction::east(), neighbor.getPressureDirection());
}

}
}
