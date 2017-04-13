#include "../game/BlobState.hpp"
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
    std::chrono::milliseconds one_second;
    P* particleAt(IntVector position) {
        for (P* particle: state.getParticles()) {
            if (particle->getPosition() == position) {
                return particle;
            }
        }
        return nullptr;
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
