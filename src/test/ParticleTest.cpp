#include "../game/Particle.hpp"
#include "../Config.hpp"
#include "TestData.hpp"

#include <chrono>

namespace wotmin2d {
namespace test {

class ParticleTest : public ::testing::Test {
    protected:
    ParticleTest() :
        start_position(5, 5),
        td(),
        one_second(std::chrono::milliseconds(1000)) {}
    IntVector start_position;
    TestData<Particle> td;
    const std::chrono::milliseconds one_second;
    void callParticleMove(Particle& particle, Direction direction) {
        particle.move({}, direction);
    }
};

TEST_F(ParticleTest, recognizesPathToItself) {
    td.makeParticles({}, { td.inside });
    EXPECT_TRUE(td.particles.front()->hasPath({}));
}

TEST_F(ParticleTest, recognizesPathsThatExist) {
    td.makeParticles({ td.lineA, td.lineB }, {});
    const Particle* beforeCorner = td.particle_map[IntVector(3, 9)];
    EXPECT_TRUE(beforeCorner->hasPath({ Direction::north() }));
    EXPECT_TRUE(beforeCorner->hasPath({ Direction::north(),
                                        Direction::east() }));
}

TEST_F(ParticleTest, recognizesPathsOnLoop) {
    td.makeParticles({ td.loop }, {});
    const Particle* onLoop = td.particle_map[IntVector(10, 12)];
    EXPECT_TRUE(onLoop->hasPath({ Direction::north(),
                                  Direction::east(),
                                  Direction::east(),
                                  Direction::south(),
                                  Direction::south(),
                                  Direction::west(),
                                  Direction::west(),
                                  Direction::north() }));
}

TEST_F(ParticleTest, recognizesThatPathsDontExist) {
    td.makeParticles({ td.loop }, {});
    const Particle* onLoop = td.particle_map[IntVector(10, 12)];
    EXPECT_FALSE(onLoop->hasPath({ Direction::east() }));
    EXPECT_FALSE(onLoop->hasPath({ Direction::west() }));
}

TEST_F(ParticleTest, moves) {
    Particle p(td.inside);
    IntVector before = p.getPosition();
    p.setTarget(p.getPosition() + Direction::north().vector(),
                Config::min_directed_movement_pressure);
    p.advance(one_second);
    callParticleMove(p, Direction::north());
    EXPECT_EQ(before + Direction::north().vector(), p.getPosition());
    before = p.getPosition();
    p.setTarget(p.getPosition() + Direction::east().vector(),
                Config::min_directed_movement_pressure);
    p.advance(one_second);
    callParticleMove(p, Direction::east());
    EXPECT_EQ(before + Direction::east().vector(), p.getPosition());
}

TEST_F(ParticleTest, doesntWantToMoveInitially) {
    Particle p(start_position);
    EXPECT_FALSE(p.canMove());
    EXPECT_EQ(FloatVector(0.0f, 0.0f), p.getPressure());
}

TEST_F(ParticleTest, doesntWantToMoveWithTargetBeforeAdvancing) {
    Particle p(start_position);
    p.setTarget(start_position + IntVector(5, 0), 1.0f);
    EXPECT_FALSE(p.canMove());
    EXPECT_EQ(FloatVector(0.0f, 0.0f), p.getPressure());
}

TEST_F(ParticleTest, wantsToMoveAfterAdvancing) {
    Particle p(start_position);
    p.setTarget(start_position + IntVector(5, 0), 1.0f);
    p.advance(one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::east(), p.getPressureDirection());
    EXPECT_GT(p.getPressure().dot(FloatVector(1.0f, 0.0f)), 0.0f);
}

TEST_F(ParticleTest, canChangeDirection) {
    Particle p(start_position);
    p.setTarget(start_position + IntVector(5, 0), 1.0f);
    p.advance(one_second);
    EXPECT_TRUE(p.canMove());
    callParticleMove(p, Direction::east());
    EXPECT_EQ(start_position + Direction::east().vector(), p.getPosition());
    p.setTarget(start_position - IntVector(5, 0), 1.0f);
    p.advance(one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::west(), p.getPressureDirection());
    EXPECT_GT(p.getPressure().dot(FloatVector(-1.0f, 0.0f)), 0.0f);
}

TEST_F(ParticleTest, canMoveInNonStraightLines) {
    Particle p(start_position);
    p.setTarget(start_position + IntVector(2, 1), 1.0f);
    p.advance(2 * one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::east(), p.getPressureDirection());
    EXPECT_GT(p.getPressure().dot(FloatVector(1.0f, 0.0f)), 0.0f);
    callParticleMove(p, Direction::east());
    EXPECT_EQ(start_position + Direction::east().vector(), p.getPosition());
    p.advance(one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::north(), p.getPressureDirection());
    EXPECT_GT(p.getPressure().dot(FloatVector(0.0f, 1.0f)), 0.0f);
    callParticleMove(p, Direction::north());
    EXPECT_EQ(start_position + Direction::east().vector()
                  + Direction::north().vector(),
              p.getPosition());
}

TEST_F(ParticleTest,
       continuesMovingWithoutTargetWithRemainingPressureUntilItRunsOut) {
    Particle p(start_position);
    p.setTarget(start_position + IntVector(1, 0), 2.0f);
    p.advance(one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::east(), p.getPressureDirection());
    callParticleMove(p, Direction::east());
    EXPECT_EQ(start_position + Direction::east().vector(), p.getPosition());
    p.setTarget(start_position, 0.0f);
    p.advance(one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::east(), p.getPressureDirection());
    EXPECT_GT(p.getPressure().dot(FloatVector(1.0f, 0.0f)), 0.0f);
    callParticleMove(p, Direction::east());
    EXPECT_EQ(start_position + (Direction::east().vector() * 2),
              p.getPosition());
    p.advance(one_second);
    EXPECT_FALSE(p.canMove());
    EXPECT_EQ(FloatVector(0.0f, 0.0f), p.getPressure());
}

TEST_F(ParticleTest, passesOnPartOfPressureOnCollision) {
    Particle p(start_position);
    Particle neighbor(start_position + IntVector(1, 0));;
    p.setTarget(start_position + IntVector(5, 0), 1.0f);
    p.advance(one_second);
    neighbor.advance(one_second);
    FloatVector p_pressure_before = p.getPressure();
    FloatVector neighbor_pressure_before = neighbor.getPressure();
    p.collideWith(neighbor, Direction::east());
    EXPECT_FALSE(p.canMove());
    EXPECT_EQ(p_pressure_before * (1.0f - Config::collision_pass_on),
              p.getPressure());
    // For some reason, we have to specify that collision_pass_on is a float
    // here, or the compiler complains.
    EXPECT_EQ((p_pressure_before * (float)Config::collision_pass_on)
                  + neighbor_pressure_before,
              neighbor.getPressure());
    EXPECT_EQ(Direction::east(), neighbor.getPressureDirection());
}

}
}
