#include "../game/ParticlePressureState.hpp"
#include "../game/Vector.hpp"
#include "../game/Direction.hpp"

#include <gtest/gtest.h>

namespace wotmin2d {
namespace test {

using Movement = ParticlePressureState::Movement;

class ParticlePressureStateTest : public ::testing::Test {
    protected:
    ParticlePressureStateTest() :
        start_position(5, 5) {}
    IntVector start_position;
};

TEST_F(ParticlePressureStateTest, doesntWantToMoveInitially) {
    ParticlePressureState pps;
    Movement movement = pps.getMovement();
    EXPECT_FALSE(movement.second);
}

TEST_F(ParticlePressureStateTest, doesntWantToMoveWithTargetBeforeAdvancing) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(5, 0), 1.0f);
    Movement movement = pps.getMovement();
    EXPECT_FALSE(movement.second);
}

TEST_F(ParticlePressureStateTest, wantsToMoveAfterAdvancing) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(5, 0), 1.0f);
    pps.advance(start_position);
    Movement movement = pps.getMovement();
    EXPECT_EQ(Direction::east(), movement.first);
    EXPECT_TRUE(movement.second);
}

TEST_F(ParticlePressureStateTest, canChangeDirection) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(5, 0), 1.0f);
    pps.advance(start_position);
    pps.updatePressureAfterMovement(Direction::east().vector());
    pps.setTarget(start_position - IntVector(5, 0), 1.0f);
    pps.advance(start_position + Direction::east().vector());
    Movement movement = pps.getMovement();
    EXPECT_EQ(Direction::west(), movement.first);
    EXPECT_TRUE(movement.second);
}

TEST_F(ParticlePressureStateTest, canMoveInNonStraightLines) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(2, 1), 1.0f);
    pps.advance(start_position);
    Movement movement = pps.getMovement();
    EXPECT_EQ(Direction::east(), movement.first);
    EXPECT_TRUE(movement.second);
    pps.updatePressureAfterMovement(Direction::east().vector());
    pps.advance(start_position + Direction::east().vector());
    movement = pps.getMovement();
    EXPECT_EQ(Direction::north(), movement.first);
    EXPECT_TRUE(movement.second);
}

TEST_F(ParticlePressureStateTest,
       continuesMovingWithoutTargetWithRemainingPressureUntilItRunsOut) {
    ParticlePressureState pps;
    pps.setTarget(start_position + IntVector(1, 0), 2.0f);
    pps.advance(start_position);
    pps.updatePressureAfterMovement(Direction::east().vector());
    pps.setTarget(start_position, 0.0f);
    pps.advance(start_position + Direction::east().vector());
    Movement movement = pps.getMovement();
    EXPECT_EQ(Direction::east(), movement.first);
    EXPECT_TRUE(movement.second);
    pps.updatePressureAfterMovement(Direction::east().vector());
    pps.advance(start_position + Direction::east().vector() * 2);
    movement = pps.getMovement();
    EXPECT_FALSE(movement.second);
}

TEST_F(ParticlePressureStateTest, passesOnEntirePressureOnCollision) {
    ParticlePressureState pps;
    ParticlePressureState neighbor;
    pps.setTarget(start_position + IntVector(5, 0), 1.0f);
    pps.advance(start_position);
    neighbor.advance(start_position + IntVector(1, 0));
    pps.collideWith(neighbor);
    Movement pps_movement = pps.getMovement();
    Movement neighbor_movement = neighbor.getMovement();
    EXPECT_FALSE(pps_movement.second);
    EXPECT_TRUE(neighbor_movement.second);
    EXPECT_EQ(Direction::east(), neighbor_movement.first);
}

}
}
