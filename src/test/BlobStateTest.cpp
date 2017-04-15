#include "../game/BlobState.hpp"
#include "../game/Particle.hpp"
#include "mock/MockParticle.hpp"
#include "../game/Vector.hpp"
#include "../Config.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

namespace wotmin2d {
namespace test {

using ::testing::_;
using ::testing::Ref;
using ::testing::ReturnRefOfCopy;
using ::testing::Return;
using ::testing::UnorderedElementsAre;

using P = mock::NiceMockParticle;

class BlobStateTest : public ::testing::Test {
    protected:
    BlobStateTest():
        state(),
        one_second(1000) {}
    BlobState<P> state;
    BlobState<Particle> real_state;
    std::chrono::milliseconds one_second;
    template<class PT>
    PT* particleAt(IntVector position, BlobState<PT>& state) {
        for (PT* particle: state.getParticles()) {
            if (particle->getPosition() == position) {
                return particle;
            }
        }
        return nullptr;
    }
    P* particleAt(IntVector position) {
        return particleAt(position, state);
    }
    Particle* realParticleAt(IntVector position) {
        return particleAt(position, real_state);
    }
};

TEST_F(BlobStateTest, addsParticles) {
    IntVector pos1(0, 0);
    IntVector pos2(5, 7);
    state.addParticle(pos1);
    ASSERT_EQ(1, state.getParticles().size());
    EXPECT_EQ(pos1, state.getParticles().front()->getPosition());
    state.addParticle(pos2);
    EXPECT_EQ(2, state.getParticles().size());
    EXPECT_EQ(pos2, state.getParticles().back()->getPosition());
}

TEST_F(BlobStateTest, advancesAllParticles) {
    IntVector pos1(0, 0);
    IntVector pos2(0, 1);
    IntVector pos3(5, 7);
    state.addParticle(pos1);
    state.addParticle(pos2);
    state.addParticle(pos3);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    P* particle3 = particleAt(pos3);
    EXPECT_CALL(*particle1, advance(one_second)).Times(1);
    EXPECT_CALL(*particle2, advance(one_second)).Times(1);
    EXPECT_CALL(*particle3, advance(one_second)).Times(1);
    state.advanceParticles(one_second);
}

TEST_F(BlobStateTest, setsNoNeighborsOnIsolatedParticles) {
    IntVector pos1(0, 0);
    IntVector pos2(5, 7);
    state.addParticle(pos1);
    state.addParticle(pos2);
    for (Direction direction: Direction::all()) {
        P* particle1 = particleAt(pos1);
        P* particle2 = particleAt(pos2);
        EXPECT_EQ(nullptr, particle1->getNeighbor(direction));
        EXPECT_EQ(nullptr, particle2->getNeighbor(direction));
    }
}

TEST_F(BlobStateTest, setsNeighbors) {
    IntVector pos1(0, 0);
    IntVector pos2(0, 1);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    EXPECT_EQ(particle1, particle2->getNeighbor(Direction::south()));
    EXPECT_EQ(particle2, particle1->getNeighbor(Direction::north()));
}

TEST_F(BlobStateTest, movesParticles) {
    IntVector pos1(0, 0);
    state.addParticle(pos1);
    P* particle1 = particleAt(pos1);
    particle1->setTarget(pos1 + Direction::east().vector(),
                         2.0f * Config::min_directed_movement_pressure);
    particle1->advance(one_second);
    state.moveParticle(*particle1, Direction::east());
    EXPECT_EQ(pos1 + Direction::east().vector(), particle1->getPosition());
}

TEST_F(BlobStateTest, unsetsNeighborsAfterMoving) {
    IntVector pos1(0, 0);
    IntVector pos2(0, 1);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    particle1->setTarget(pos1 + Direction::east().vector(),
                         2.0f * Config::min_directed_movement_pressure);
    particle1->advance(one_second);
    state.moveParticle(*particle1, Direction::east());
    for (Direction direction: Direction::all()) {
        EXPECT_EQ(nullptr, particle1->getNeighbor(direction));
        EXPECT_EQ(nullptr, particle2->getNeighbor(direction));
    }
}

TEST_F(BlobStateTest, setsNeighborsAfterMoving) {
    IntVector pos1(0, 0);
    IntVector pos2(1, 1);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    particle1->setTarget(pos1 + Direction::east().vector(),
                         2.0f * Config::min_directed_movement_pressure);
    particle1->advance(one_second);
    state.moveParticle(*particle1, Direction::east());
    EXPECT_EQ(particle1, particle2->getNeighbor(Direction::south()));
    EXPECT_EQ(particle2, particle1->getNeighbor(Direction::north()));
}

TEST_F(BlobStateTest, collidesParticles) {
    IntVector pos1(0, 0);
    IntVector pos2(0, 1);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    EXPECT_CALL(*particle1, collideWith(Ref(*particle2), Direction::north()));
    state.collideParticles(*particle1, *particle2, Direction::north());
}

TEST_F(BlobStateTest, killsPressureOnCollisionWithWall) {
    IntVector pos1(0, 0);
    IntVector pos2(5, 7);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    EXPECT_CALL(*particle1, killPressureInDirection(Direction::north()));
    EXPECT_CALL(*particle2, killPressureInDirection(Direction::east()));
    state.collideParticleWithWall(*particle1, Direction::north());
    state.collideParticleWithWall(*particle2, Direction::east());
}

TEST_F(BlobStateTest, addsFollowers) {
    IntVector pos1(0, 0);
    IntVector pos2(0, 1);
    IntVector pos3(5, 7);
    state.addParticle(pos1);
    state.addParticle(pos2);
    state.addParticle(pos3);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    P* particle3 = particleAt(pos3);
    std::vector<P*> followers = { particle2, particle3 };
    EXPECT_CALL(*particle1, addFollowers(_, followers));
    state.addParticleFollowers(*particle1, followers);
}

TEST_F(BlobStateTest, getsNullAsHighestMobilityOnEmpty) {
    EXPECT_EQ(nullptr, state.getHighestMobilityParticle());
}

TEST_F(BlobStateTest, getsHighestMobilityParticle) {
    IntVector pos1(0, 0);
    IntVector pos2(0, 1);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    ON_CALL(*particle2, canMove()).WillByDefault(Return(true));
    // Only particle 2 can even move.
    EXPECT_EQ(particle2, state.getHighestMobilityParticle());
    ON_CALL(*particle1, canMove()).WillByDefault(Return(true));
    ON_CALL(*particle1, getPressure())
        .WillByDefault(ReturnRefOfCopy(FloatVector(1.0f, 0.0f)));
    // Particle 1 has more pressure.
    EXPECT_EQ(particle1, state.getHighestMobilityParticle());
    ON_CALL(*particle2, getPressure())
        .WillByDefault(ReturnRefOfCopy(FloatVector(-1.0f, 0.5f)));
    // Now particle 2.
    EXPECT_EQ(particle2, state.getHighestMobilityParticle());
}

TEST_F(BlobStateTest, getsHighestMobilityParticleAfterAdvances) {
    IntVector pos1(3, 5);
    IntVector pos2(5, 7);
    real_state.addParticle(pos1);
    real_state.addParticle(pos2);
    Particle* particle1 = realParticleAt(pos1);
    Particle* particle2 = realParticleAt(pos2);
    particle1->setTarget(particle1->getPosition()
                             + Direction::north().vector() * 100,
                         Config::min_directed_movement_pressure * 3.0f);
    particle2->setTarget(particle2->getPosition()
                             + Direction::north().vector() * 100,
                         Config::min_directed_movement_pressure * 2.0f);
    particle1->advance(one_second);
    particle2->advance(one_second);
    EXPECT_EQ(particle1, real_state.getHighestMobilityParticle());
    particle1->setTarget(particle1->getPosition(), 0.0f);
    particle1->advance(one_second);
    particle2->advance(one_second);
    EXPECT_EQ(particle2, real_state.getHighestMobilityParticle());
}

TEST_F(BlobStateTest, getsHighestMobilityParticleAfterMove) {
    IntVector pos1(3, 5);
    IntVector pos2(5, 7);
    real_state.addParticle(pos1);
    real_state.addParticle(pos2);
    Particle* particle1 = realParticleAt(pos1);
    Particle* particle2 = realParticleAt(pos2);
    particle1->setTarget(particle1->getPosition()
                             + Direction::north().vector() * 100,
                         Config::min_directed_movement_pressure * 3.0f + 3.0f);
    particle2->setTarget(particle2->getPosition()
                             + Direction::north().vector() * 100,
                         Config::min_directed_movement_pressure * 2.0f + 3.0f);
    particle1->advance(one_second);
    particle2->advance(one_second);
    ASSERT_LT(particle2->getPressure().squaredNorm(),
              particle1->getPressure().squaredNorm());
    int num_moves
        = static_cast<int>(Config::min_directed_movement_pressure) + 1;
    for (int i = 0; i < num_moves; i++) {
        ASSERT_TRUE(particle1->canMove());
        real_state.moveParticle(*particle1, particle1->getPressureDirection());
    }
    EXPECT_EQ(particle2, real_state.getHighestMobilityParticle());
    EXPECT_TRUE(particle2->canMove());
}

TEST_F(BlobStateTest, getsHighestMobilityParticleAfterLeaderAdvance) {
    if (Config::target_pressure_share == 0.0f) {
        // No point in this test since leaders aren't supposed to share with
        // their followers.
        return;
    }
    ASSERT_GT((float)Config::target_pressure_share, 0.0f);
    ASSERT_LE((float)Config::target_pressure_share, 1.0f);
    IntVector pos1(3, 5);
    IntVector pos2(5, 7);
    IntVector pos3(7, 11);
    real_state.addParticle(pos1);
    real_state.addParticle(pos2);
    real_state.addParticle(pos3);
    Particle* particle1 = realParticleAt(pos1);
    Particle* particle2 = realParticleAt(pos2);
    Particle* particle3 = realParticleAt(pos3);
    particle1->setTarget(particle1->getPosition()
                             + Direction::north().vector() * 100,
                         Config::min_directed_movement_pressure);
    particle3->setTarget(particle3->getPosition()
                             + Direction::north().vector() * 100,
                         Config::min_directed_movement_pressure * 2.0f);
    particle1->advance(one_second);
    particle3->advance(one_second);
    real_state.addParticleFollowers(*particle1, { particle2 });
    ASSERT_LT(particle2->getPressure().squaredNorm(),
              particle3->getPressure().squaredNorm());
    // Particle 2 has lower mobility than particle 3, but advancing particle 1
    // should change that eventually since it's a leader.
    int num_advances = static_cast<int>(2.0f / Config::target_pressure_share)
                       + 1;
    particle1->advance(one_second * num_advances);
    Particle* highest_mobility;
    while (true) {
        highest_mobility = real_state.getHighestMobilityParticle();
        if (highest_mobility == particle1) {
            ASSERT_TRUE(particle1->canMove());
            real_state.moveParticle(*particle1,
                                    particle1->getPressureDirection());
        } else {
            break;
        }
    }
    EXPECT_EQ(particle2, highest_mobility);
    EXPECT_TRUE(particle2->canMove());
    EXPECT_TRUE(particle3->canMove());
}

TEST_F(BlobStateTest, getsHighestMobilityParticleAfterFollowerAdd) {
    if (Config::boost_fraction == 0.0f) {
        // No point in this test since leaders aren't supposed to boost
        // followers when they're added.
        return;
    }
    ASSERT_GT((float)Config::boost_fraction, 0.0f);
    ASSERT_LE((float)Config::boost_fraction, 1.0f);
    IntVector pos1(3, 5);
    IntVector pos2(5, 7);
    IntVector pos3(7, 11);
    real_state.addParticle(pos1);
    real_state.addParticle(pos2);
    real_state.addParticle(pos3);
    Particle* particle1 = realParticleAt(pos1);
    Particle* particle2 = realParticleAt(pos2);
    Particle* particle3 = realParticleAt(pos3);
    float pressure3 = Config::min_directed_movement_pressure * 2.0f;
    float pressure1_mult = static_cast<int>(1.0f / Config::boost_fraction) + 1;
    particle1->setTarget(particle1->getPosition()
                             + Direction::north().vector() * 100,
                         pressure1_mult * pressure3);
    particle3->setTarget(particle3->getPosition()
                             + Direction::north().vector() * 100,
                         pressure3);
    particle1->advance(one_second);
    particle3->advance(one_second);
    ASSERT_LT(particle2->getPressure().squaredNorm(),
              particle3->getPressure().squaredNorm());
    // Particle 2 has lower mobility than particle 3, but adding it as a
    // follower to particle 1 should change that since it will receive a boost.
    real_state.addParticleFollowers(*particle1, { particle2 });
    Particle* highest_mobility;
    while (true) {
        highest_mobility = real_state.getHighestMobilityParticle();
        if (highest_mobility == particle1) {
            ASSERT_TRUE(particle1->canMove());
            real_state.moveParticle(*particle1,
                                    particle1->getPressureDirection());
        } else {
            break;
        }
    }
    EXPECT_EQ(particle2, highest_mobility);
    EXPECT_TRUE(particle2->canMove());
    EXPECT_TRUE(particle3->canMove());
}

TEST_F(BlobStateTest, getsParticlesAroundCenter) {
    IntVector center(5, 5);
    IntVector pos1 = center + IntVector(0, 2);
    IntVector pos2 = center + IntVector(-1, 1);
    IntVector pos3 = center + IntVector(-2, -2);
    state.addParticle(center);
    state.addParticle(pos1);
    state.addParticle(pos2);
    state.addParticle(pos3);
    P* particle_c = particleAt(center);
    P* particle_1 = particleAt(pos1);
    P* particle_2 = particleAt(pos2);
    P* particle_3 = particleAt(pos3);
    EXPECT_THAT(state.getParticles(center, 0.0f),
                UnorderedElementsAre(particle_c));
    EXPECT_THAT(state.getParticles(center, 1.9f),
                UnorderedElementsAre(particle_c, particle_2));
    EXPECT_THAT(state.getParticles(center, 2.0f),
                UnorderedElementsAre(particle_c, particle_2, particle_1));
    EXPECT_THAT(state.getParticles(center, 3.0f),
                UnorderedElementsAre(particle_c, particle_2, particle_1,
                                     particle_3));
}

}
}
