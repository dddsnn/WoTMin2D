#include "../game/Particle.hpp"
#include "../Config.hpp"
#include "TestData.hpp"

#include <chrono>

namespace wotmin2d {
namespace test {

using ::testing::FloatEq;

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
    void callAddFollowers(Particle& particle,
                          std::vector<Particle*> followers)
    {
        particle.addFollowers({}, followers);
    }
    void callAdvance(Particle& particle, std::chrono::milliseconds time_delta) {
        particle.advance({}, time_delta);
    }
    void callCollideWith(Particle& first, Particle& second,
                         Direction collision_direction) {
        first.collideWith({}, second, collision_direction);
    }
    void callKillPressure(Particle& particle, Direction direction) {
        particle.killPressureInDirection({}, direction);
    }
    void callDamage(Particle& particle, unsigned int amount) {
        particle.damage({}, amount);
    }
};

TEST_F(ParticleTest, hasNeighbors) {
    td.makeParticles({ td.lineA, td.block }, { td.inside });
    EXPECT_FALSE(td.particle_map[td.inside]->hasNeighbor());
    EXPECT_TRUE(td.particle_map[IntVector(3, 10)]->hasNeighbor());
    EXPECT_TRUE(td.particle_map[IntVector(3, 4)]->hasNeighbor());
    EXPECT_TRUE(td.particle_map[IntVector(5, 4)]->hasNeighbor());
    EXPECT_TRUE(td.particle_map[IntVector(5, 3)]->hasNeighbor());
}

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

TEST_F(ParticleTest, reportsCorrectPressureDirection) {
    Particle p_north(td.inside);
    p_north.setTarget(p_north.getPosition() + Direction::north().vector(),
                     Config::min_directed_movement_pressure);
    callAdvance(p_north, 2 * one_second);
    EXPECT_EQ(Direction::north(), p_north.getPressureDirection());
    Particle p_east(td.inside);
    p_east.setTarget(p_east.getPosition() + Direction::east().vector(),
                     Config::min_directed_movement_pressure);
    callAdvance(p_east, 2 * one_second);
    EXPECT_EQ(Direction::east(), p_east.getPressureDirection());
    Particle p_ssw(td.inside);
    p_ssw.setTarget(p_ssw.getPosition() + IntVector(-1, -2),
                     Config::min_directed_movement_pressure);
    callAdvance(p_ssw, 2 * one_second);
    EXPECT_EQ(Direction::south(), p_ssw.getPressureDirection());
}

TEST_F(ParticleTest, moves) {
    Particle p(td.inside);
    IntVector before = p.getPosition();
    p.setTarget(p.getPosition() + Direction::north().vector(),
                Config::min_directed_movement_pressure);
    callAdvance(p, one_second);
    callParticleMove(p, Direction::north());
    EXPECT_EQ(before + Direction::north().vector(), p.getPosition());
    before = p.getPosition();
    p.setTarget(p.getPosition() + Direction::east().vector(),
                Config::min_directed_movement_pressure);
    callAdvance(p, one_second);
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

TEST_F(ParticleTest, doesntWantToMoveWithTargetAtOwnPosition) {
    Particle p(start_position);
    p.setTarget(p.getPosition(), 2 * Config::min_directed_movement_pressure);
    callAdvance(p, one_second);
    EXPECT_FALSE(p.canMove());
    EXPECT_EQ(FloatVector(0.0f, 0.0f), p.getPressure());
}

TEST_F(ParticleTest, wantsToMoveAfterAdvancing) {
    Particle p(start_position);
    p.setTarget(start_position + IntVector(5, 0), 1.0f);
    callAdvance(p, one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::east(), p.getPressureDirection());
    EXPECT_GT(p.getPressure().dot(FloatVector(1.0f, 0.0f)), 0.0f);
}

TEST_F(ParticleTest, canChangeDirection) {
    Particle p(start_position);
    p.setTarget(start_position + IntVector(5, 0), 1.0f);
    callAdvance(p, one_second);
    EXPECT_TRUE(p.canMove());
    callParticleMove(p, Direction::east());
    EXPECT_EQ(start_position + Direction::east().vector(), p.getPosition());
    p.setTarget(start_position - IntVector(5, 0), 1.0f);
    callAdvance(p, one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::west(), p.getPressureDirection());
    EXPECT_GT(p.getPressure().dot(FloatVector(-1.0f, 0.0f)), 0.0f);
}

TEST_F(ParticleTest, canMoveInNonStraightLines) {
    Particle p(start_position);
    p.setTarget(start_position + IntVector(2, 1), 1.0f);
    callAdvance(p, 2 * one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::east(), p.getPressureDirection());
    EXPECT_GT(p.getPressure().dot(FloatVector(1.0f, 0.0f)), 0.0f);
    callParticleMove(p, Direction::east());
    EXPECT_EQ(start_position + Direction::east().vector(), p.getPosition());
    callAdvance(p, one_second);
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
    callAdvance(p, one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::east(), p.getPressureDirection());
    callParticleMove(p, Direction::east());
    EXPECT_EQ(start_position + Direction::east().vector(), p.getPosition());
    p.setTarget(start_position, 0.0f);
    callAdvance(p, one_second);
    EXPECT_TRUE(p.canMove());
    EXPECT_EQ(Direction::east(), p.getPressureDirection());
    EXPECT_GT(p.getPressure().dot(FloatVector(1.0f, 0.0f)), 0.0f);
    callParticleMove(p, Direction::east());
    EXPECT_EQ(start_position + (Direction::east().vector() * 2),
              p.getPosition());
    callAdvance(p, one_second);
    EXPECT_FALSE(p.canMove());
    EXPECT_EQ(FloatVector(0.0f, 0.0f), p.getPressure());
}

TEST_F(ParticleTest, passesOnPartOfPressureOnCollision) {
    Particle p(start_position);
    Particle neighbor(start_position + IntVector(1, 0));
    p.setTarget(start_position + IntVector(5, 0), 1.0f);
    callAdvance(p, one_second);
    callAdvance(neighbor, one_second);
    FloatVector p_pressure_before = p.getPressure();
    FloatVector neighbor_pressure_before = neighbor.getPressure();
    callCollideWith(p, neighbor, Direction::east());
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

TEST_F(ParticleTest, killsPressure) {
    for (Direction direction: Direction::all()) {
        Particle p(start_position);
        p.setTarget(p.getPosition() + direction.vector(), 1.0f);
        callAdvance(p, one_second);
        EXPECT_NE(FloatVector(0.0f, 0.0f), p.getPressure());
        callKillPressure(p, direction);
        EXPECT_EQ(FloatVector(0.0f, 0.0f), p.getPressure());
    }
}

TEST_F(ParticleTest, killsPressureOnlyInDirectionThatWasAsked) {
    Particle p1(start_position);
    Particle p2(start_position + IntVector(1, 0));
    p1.setTarget(p1.getPosition() + Direction::north().vector(), 1.0f);
    p2.setTarget(p2.getPosition() + IntVector(4, 7), 1.0f);
    callAdvance(p1, one_second);
    callAdvance(p2, one_second);
    FloatVector p1_pressure_before = p1.getPressure();
    FloatVector p2_pressure_before = p2.getPressure();
    EXPECT_NE(FloatVector(0.0f, 0.0f), p1_pressure_before);
    EXPECT_NE(FloatVector(0.0f, 0.0f), p2_pressure_before);
    callKillPressure(p1, Direction::west());
    callKillPressure(p2, Direction::east());
    EXPECT_EQ(p1_pressure_before, p1.getPressure());
    FloatVector expected_p2(0.0f, p2_pressure_before.getY());
    EXPECT_EQ(expected_p2, p2.getPressure());
}

TEST_F(ParticleTest, addsFollowersWhenAskedAndBoostsThem) {
    Particle p(td.inside);
    Particle f1(td.onSouthBorder);
    Particle f2(td.inSouthWestCorner);
    p.setTarget(td.inside + Direction::north().vector() * 5,
                5.0f * Config::min_directed_movement_pressure);
    callAdvance(p, one_second);
    FloatVector before_pressure = p.getPressure();
    callAddFollowers(p, { &f1, &f2 });
    float follower_magnitude = before_pressure.norm() * Config::boost_fraction;
    FloatVector f1_p
        = static_cast<FloatVector>(p.getPosition() - f1.getPosition());
    FloatVector f2_p
        = static_cast<FloatVector>(p.getPosition() - f2.getPosition());
    FloatVector expected_1 = f1_p * 0.5f * follower_magnitude / f1_p.norm();
    FloatVector expected_2 = f2_p * 0.5f * follower_magnitude / f2_p.norm();
    FloatVector expected_p = before_pressure * (1.0f - Config::boost_fraction);
    // TODO Write some proper matchers for FloatVector. We need FloatEq because
    // there seem to be (unimportant) rounding errors.
    EXPECT_THAT(f1.getPressure().getX(), FloatEq(expected_1.getX()));
    EXPECT_THAT(f1.getPressure().getY(), FloatEq(expected_1.getY()));
    EXPECT_THAT(f2.getPressure().getX(), FloatEq(expected_2.getX()));
    EXPECT_THAT(f2.getPressure().getY(), FloatEq(expected_2.getY()));
    EXPECT_THAT(p.getPressure().getX(), FloatEq(expected_p.getX()));
    EXPECT_THAT(p.getPressure().getY(), FloatEq(expected_p.getY()));
}

TEST_F(ParticleTest, sharesPressureWithFollowersOnAdvance) {
    Particle p(td.inside);
    Particle f1(td.onSouthBorder);
    Particle f2(td.inSouthWestCorner);
    callAddFollowers(p, { &f1, &f2 });
    p.setTarget(td.inside + Direction::north().vector() * 5,
                5.0f * Config::min_directed_movement_pressure);
    callAdvance(p, one_second);
    float p_magnitude = p.getPressure().norm();
    float f1_magnitude = f1.getPressure().norm();
    float f2_magnitude = f2.getPressure().norm();
    float p_multiplier = (2.0f + Config::target_pressure_share) / 3.0f;
    EXPECT_THAT(f1_magnitude, FloatEq(f2_magnitude));
    EXPECT_THAT(p_magnitude, FloatEq(f1_magnitude * p_multiplier));
    // Also check the added pressure is pointing towards the leader.
    FloatVector f1_to_p
        = static_cast<FloatVector>(p.getPosition() - f1.getPosition());
    FloatVector f2_to_p
        = static_cast<FloatVector>(p.getPosition() - f2.getPosition());
    float expected_f1_direction = f1_to_p.getX() / f1_to_p.getY();
    float expected_f2_direction = f2_to_p.getX() / f2_to_p.getY();
    float f1_direction = f1.getPressure().getX() / f1.getPressure().getY();
    float f2_direction = f2.getPressure().getX() / f2.getPressure().getY();
    EXPECT_THAT(expected_f1_direction, FloatEq(f1_direction));
    EXPECT_THAT(expected_f2_direction, FloatEq(f2_direction));
}

TEST_F(ParticleTest, passesOnLeadersOnCollision) {
    Particle p1(td.inside);
    Particle p2(td.onSouthBorder + Direction::north().vector());
    Particle f(td.onSouthBorder);
    callAddFollowers(p1, { &f });
    p1.setTarget(td.inside + Direction::north().vector() * 5,
                 5.0f * Config::min_directed_movement_pressure);
    callCollideWith(f, p2, Direction::north());
    callAdvance(p1, one_second);
    float p1_magnitude = p1.getPressure().norm();
    float p2_magnitude = p2.getPressure().norm();
    float p1_multiplier = (1.0f + Config::target_pressure_share) / 2.0f;
    EXPECT_THAT(p2_magnitude, FloatEq(p1_magnitude * p1_multiplier));
    EXPECT_EQ(FloatVector(0.0f, 0.0f), f.getPressure());
}

TEST_F(ParticleTest, dropsFollowershipOnCollisionWithLeader) {
    Particle p(td.inside);
    Particle f(td.inside + Direction::south().vector());
    callAddFollowers(p, { &f });
    p.setTarget(td.inside + Direction::north().vector() * 5,
                5.0f * Config::min_directed_movement_pressure);
    callCollideWith(f, p, Direction::north());
    callAdvance(p, one_second);
    EXPECT_EQ(FloatVector(0.0f, 0.0f), f.getPressure());
}

TEST_F(ParticleTest, dropsFollowershipOnOpposingPressureDirections) {
    Particle p(td.inside);
    Particle f(td.inside + Direction::west().vector() * 2);
    float pressure = 5.0f * Config::min_directed_movement_pressure;
    IntVector p_pressure_direction(1, 4);
    IntVector f_pressure_direction(-4, 0);
    p.setTarget(p.getPosition() + p_pressure_direction, pressure);
    f.setTarget(f.getPosition() + f_pressure_direction, pressure);
    callAdvance(p, one_second);
    callAdvance(f, one_second);
    float boost_magnitude = p.getPressure().norm() * Config::boost_fraction;
    FloatVector f_to_p
        = static_cast<FloatVector>(p.getPosition() - f.getPosition());
    FloatVector boost
        = f_to_p * boost_magnitude / f_to_p.norm();
    callAddFollowers(p, { &f });
    // We have to call advance() on f here first so f drops followership to p.
    // If we called p first, it would give some of its pressure to f. This isn't
    // exactly ideal, but the important thing is that followership is dropped
    // after a finite amount of time of going if opposing directions.
    callAdvance(f, one_second);
    callAdvance(p, one_second);
    FloatVector p_pressure
        = static_cast<FloatVector>(p_pressure_direction)
              * pressure / p_pressure_direction.norm();
    FloatVector f_pressure
        = static_cast<FloatVector>(f_pressure_direction)
              * pressure / f_pressure_direction.norm();
    // There's two advances for each particle. p gives off a boost though.
    FloatVector expected_p = p_pressure
                                 * (2.0f - (1.0f - Config::boost_fraction));
    FloatVector expected_f = (f_pressure * 2.0f) + boost;
    EXPECT_EQ(expected_p, p.getPressure());
    EXPECT_EQ(expected_f, f.getPressure());
}

TEST_F(ParticleTest, dropsFollowershipWhenNextToLeader) {
    Particle p(td.inside);
    Particle f(td.inside + Direction::south().vector());
    callAddFollowers(p, { &f });
    p.setTarget(td.inside + Direction::north().vector() * 5,
                5.0f * Config::min_directed_movement_pressure);
    f.setTarget(f.getPosition(), 0.0f);
    callAdvance(f, one_second);
    callAdvance(p, one_second);
    EXPECT_EQ(FloatVector(0.0f, 0.0f), f.getPressure());
}

TEST_F(ParticleTest, switchesFollowershipIfFollowerAheadOfLeader) {
    Particle p1(td.inside);
    Particle p2(td.inside + Direction::south().vector() * 2);
    float pressure = 5.0f * Config::min_directed_movement_pressure;
    p1.setTarget(p1.getPosition() + Direction::north().vector(), pressure);
    p2.setTarget(p2.getPosition() + Direction::north().vector(), pressure);
    callAddFollowers(p2, { &p1 });
    callAdvance(p1, one_second);
    callAdvance(p2, one_second);
    // p1 should have realized that it's ahead of its leader p2 and switched the
    // relationship.
    callAdvance(p1, one_second);
    callAdvance(p2, one_second);
    FloatVector pressure_vector
        = static_cast<FloatVector>(Direction::north().vector()) * pressure;
    // Each of them should have one advance for themselves, and one where p1
    // shares with p2.
    float leader_share = Config::target_pressure_share / 2.0f;
    FloatVector expected_p1 = pressure_vector * (1.0f + leader_share);
    FloatVector expected_p2 = pressure_vector * (2.0f + (1.0f - leader_share));
    EXPECT_EQ(expected_p1, p1.getPressure());
    EXPECT_EQ(expected_p2, p2.getPressure());
}

TEST_F(ParticleTest, takesDamage) {
    Particle p(td.inside);
    EXPECT_EQ(100, p.getHealth());
    callDamage(p, 0);
    EXPECT_EQ(100, p.getHealth());
    callDamage(p, 25);
    EXPECT_EQ(100 - 25, p.getHealth());
    callDamage(p, 70);
    EXPECT_EQ(100 - 95, p.getHealth());
}

TEST_F(ParticleTest, takesNoMoreDamageOnceHealthReachesZero) {
    Particle p(td.inside);
    EXPECT_EQ(100, p.getHealth());
    callDamage(p, 105);
    EXPECT_EQ(0, p.getHealth());
    callDamage(p, 70);
    EXPECT_EQ(0, p.getHealth());
}

}
}
