#include "../game/BlobState.hpp"
#include "../game/Particle.hpp"
#include "mock/MockParticle.hpp"
#include "../game/Vector.hpp"
#include "../Config.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <algorithm>
#include <unordered_set>

namespace wotmin2d {
namespace test {

using ::testing::_;
using ::testing::Ref;
using ::testing::ReturnRefOfCopy;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::UnorderedElementsAre;
using ::testing::ElementsAre;
using ::testing::IsEmpty;

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
    EXPECT_EQ(pos1, (*(state.getParticles().begin()))->getPosition());
    state.addParticle(pos2);
    EXPECT_EQ(2, state.getParticles().size());
    EXPECT_TRUE(std::any_of(state.getParticles().cbegin(),
                            state.getParticles().cend(),
                            [=](P* p) { return p->getPosition() == pos2; }));
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
    EXPECT_CALL(*particle1, advance(_, one_second)).Times(1);
    EXPECT_CALL(*particle2, advance(_, one_second)).Times(1);
    EXPECT_CALL(*particle3, advance(_, one_second)).Times(1);
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
    particle1->advance({}, one_second);
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
    particle1->advance({}, one_second);
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
    particle1->advance({}, one_second);
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
    EXPECT_CALL(*particle1, collideWith(_, Ref(*particle2),
                                        Direction::north()));
    state.collideParticles(*particle1, *particle2, Direction::north());
}

TEST_F(BlobStateTest, killsPressureOnCollisionWithWall) {
    IntVector pos1(0, 0);
    IntVector pos2(5, 7);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    EXPECT_CALL(*particle1, killPressureInDirection(_, Direction::north()));
    EXPECT_CALL(*particle2, killPressureInDirection(_, Direction::east()));
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
    IntVector pos1(3, 5);
    IntVector pos2(5, 7);
    real_state.addParticle(pos1);
    real_state.addParticle(pos2);
    Particle* particle1 = realParticleAt(pos1);
    Particle* particle2 = realParticleAt(pos2);
    particle1->setTarget(particle1->getPosition()
                             + Direction::north().vector() * 10,
                         Config::min_directed_movement_pressure);
    particle2->setTarget(particle2->getPosition()
                             + Direction::north().vector() * 10,
                         Config::min_directed_movement_pressure * 1.1f);
    real_state.advanceParticles(one_second);
    ASSERT_TRUE(particle1->canMove());
    ASSERT_TRUE(particle2->canMove());
    EXPECT_EQ(particle2, real_state.getHighestMobilityParticle());
}

TEST_F(BlobStateTest, getsHighestMobilityParticleIfOnlyOneCanMove) {
    IntVector pos1(3, 5);
    IntVector pos2(5, 7);
    real_state.addParticle(pos1);
    real_state.addParticle(pos2);
    Particle* particle1 = realParticleAt(pos1);
    Particle* particle2 = realParticleAt(pos2);
    particle2->setTarget(particle2->getPosition()
                             + Direction::north().vector() * 10,
                         Config::min_directed_movement_pressure);
    real_state.advanceParticles(one_second);
    ASSERT_FALSE(particle1->canMove());
    ASSERT_TRUE(particle2->canMove());
    EXPECT_EQ(particle2, real_state.getHighestMobilityParticle());
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
    real_state.advanceParticles(one_second);
    EXPECT_EQ(particle1, real_state.getHighestMobilityParticle());
    particle1->setTarget(particle1->getPosition(), 0.0f);
    real_state.advanceParticles(one_second);
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
    real_state.advanceParticles(one_second);
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
    real_state.advanceParticles(one_second);
    real_state.addParticleFollowers(*particle1, { particle2 });
    ASSERT_LT(particle2->getPressure().squaredNorm(),
              particle3->getPressure().squaredNorm());
    // Particle 2 has lower mobility than particle 3, but advancing particle 1
    // should change that eventually since it's a leader.
    int num_advances = static_cast<int>(2.0f / Config::target_pressure_share)
                       + 1;
    particle3->setTarget(particle3->getPosition(), 0.0f);
    real_state.advanceParticles(num_advances * one_second);
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
    real_state.advanceParticles(one_second);
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

TEST_F(BlobStateTest, getsParticlesAtPosition) {
    IntVector pos1(5, 5);
    IntVector pos2(3, 4);
    IntVector pos3(4, 4);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    EXPECT_EQ(state.getParticleAt(pos1), particle1);
    EXPECT_EQ(state.getParticleAt(pos2), particle2);
    EXPECT_EQ(state.getParticleAt(pos3), nullptr);
}

TEST_F(BlobStateTest, damagesParticles) {
    IntVector pos(5, 5);
    state.addParticle(pos);
    P* particle = particleAt(pos);
    EXPECT_CALL(*particle, damage(_, 10));
    state.damageParticle(*particle, 0);
}

TEST_F(BlobStateTest, damagesParticlesWithAdvantage) {
    IntVector pos1(5, 5);
    IntVector pos2(3, 4);
    IntVector pos3(1, 1);
    state.addParticle(pos1);
    state.addParticle(pos2);
    state.addParticle(pos3);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    P* particle3 = particleAt(pos3);
    int advantage1 = 8;
    int advantage2 = 12;
    int advantage3 = -4;
    EXPECT_CALL(*particle1, damage(_, 10 - advantage1));
    EXPECT_CALL(*particle2, damage(_, 0));
    EXPECT_CALL(*particle3, damage(_, 10 - advantage3));
    state.damageParticle(*particle1, advantage1);
    state.damageParticle(*particle2, advantage2);
    state.damageParticle(*particle3, advantage3);
}

TEST_F(BlobStateTest, removesParticlesWithNoHealthLeft) {
    IntVector pos1(5, 5);
    IntVector pos2(3, 4);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    ON_CALL(*particle1, getHealth()).WillByDefault(Return(0));
    ON_CALL(*particle2, getHealth()).WillByDefault(Return(0));
    state.damageParticle(*particle2, 0);
    EXPECT_THAT(state.getParticles(), ElementsAre(particle1));
    EXPECT_THAT(state.getParticles(pos2, 5.0f), ElementsAre(particle1));
    state.damageParticle(*particle1, 0);
    EXPECT_THAT(state.getParticles(), IsEmpty());
    EXPECT_THAT(state.getParticles(pos2, 5.0f), IsEmpty());
}

TEST_F(BlobStateTest, clearsParticleMapOnParticleRemoval) {
    IntVector pos1(5, 5);
    IntVector pos2(3, 4);
    state.addParticle(pos1);
    state.addParticle(pos2);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    ON_CALL(*particle1, getHealth()).WillByDefault(Return(0));
    ON_CALL(*particle2, getHealth()).WillByDefault(Return(0));
    state.damageParticle(*particle2, 0);
    EXPECT_EQ(state.getParticleAt(pos2), nullptr);
    EXPECT_EQ(state.getParticleAt(pos1), particle1);
    state.damageParticle(*particle1, 0);
    EXPECT_EQ(state.getParticleAt(pos1), nullptr);
}

TEST_F(BlobStateTest, clearsParticleNeighborsOnParticleRemoval) {
    IntVector pos_center(5, 5);
    IntVector pos_left(4, 5);
    IntVector pos_right(6, 5);
    IntVector pos_up(5, 6);
    IntVector pos_down(5, 4);
    state.addParticle(pos_center);
    state.addParticle(pos_left);
    state.addParticle(pos_right);
    state.addParticle(pos_up);
    state.addParticle(pos_down);
    P* center = particleAt(pos_center);
    P* left = particleAt(pos_left);
    P* right = particleAt(pos_right);
    P* up = particleAt(pos_up);
    P* down = particleAt(pos_down);
    ON_CALL(*left, getHealth()).WillByDefault(Return(0));
    ON_CALL(*center, getHealth()).WillByDefault(Return(0));
    center->setNeighbor({}, Direction::west(), left);
    left->setNeighbor({}, Direction::east(), center);
    center->setNeighbor({}, Direction::east(), right);
    right->setNeighbor({}, Direction::west(), center);
    center->setNeighbor({}, Direction::north(), up);
    up->setNeighbor({}, Direction::south(), center);
    center->setNeighbor({}, Direction::south(), down);
    down->setNeighbor({}, Direction::north(), center);
    EXPECT_TRUE(center->hasNeighbor());
    EXPECT_TRUE(left->hasNeighbor());
    EXPECT_TRUE(right->hasNeighbor());
    EXPECT_TRUE(up->hasNeighbor());
    EXPECT_TRUE(down->hasNeighbor());
    state.damageParticle(*left, 0);
    EXPECT_EQ(center->getNeighbor(Direction::west()), nullptr);
    state.damageParticle(*center, 0);
    EXPECT_FALSE(right->hasNeighbor());
    EXPECT_FALSE(up->hasNeighbor());
    EXPECT_FALSE(down->hasNeighbor());
}

TEST_F(BlobStateTest, clearsParticleLeadersOnParticleRemoval) {
    IntVector pos1(5, 5);
    IntVector pos2(3, 4);
    IntVector pos3(7, 1);
    state.addParticle(pos1);
    state.addParticle(pos2);
    state.addParticle(pos3);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    P* particle3 = particleAt(pos3);
    ON_CALL(*particle1, getHealth()).WillByDefault(Return(0));
    std::unordered_set<P*> leaders({ particle2, particle3 });
    ON_CALL(*particle1, getLeaders(_)).WillByDefault(ReturnRef(leaders));
    EXPECT_CALL(*particle2, removeFollower(_, Ref(*particle1))).Times(1);
    EXPECT_CALL(*particle3, removeFollower(_, Ref(*particle1))).Times(1);
    state.damageParticle(*particle1, 0);
}

TEST_F(BlobStateTest, clearsParticleFollowersOnParticleRemoval) {
    IntVector pos1(5, 5);
    IntVector pos2(3, 4);
    IntVector pos3(7, 1);
    state.addParticle(pos1);
    state.addParticle(pos2);
    state.addParticle(pos3);
    P* particle1 = particleAt(pos1);
    P* particle2 = particleAt(pos2);
    P* particle3 = particleAt(pos3);
    ON_CALL(*particle1, getHealth()).WillByDefault(Return(0));
    std::unordered_set<P*> followers({ particle2, particle3 });
    ON_CALL(*particle1, getFollowers(_)).WillByDefault(ReturnRef(followers));
    EXPECT_CALL(*particle2, removeLeader(_, Ref(*particle1))).Times(1);
    EXPECT_CALL(*particle3, removeLeader(_, Ref(*particle1))).Times(1);
    state.damageParticle(*particle1, 0);
}

TEST_F(BlobStateTest, calculatesParticleStrength) {
    IntVector pos_center(5, 5);
    IntVector pos_next_to(4, 5);
    IntVector pos_at_border(5, 9);
    IntVector pos_outside_x(0, 5);
    IntVector pos_outside_y(5, 10);
    state.addParticle(pos_center);
    P* center = particleAt(pos_center);
    EXPECT_EQ(1, state.getParticleStrength(*center));
    state.addParticle(pos_outside_x);
    EXPECT_EQ(1, state.getParticleStrength(*center));
    state.addParticle(pos_outside_y);
    EXPECT_EQ(1, state.getParticleStrength(*center));
    state.addParticle(pos_next_to);
    EXPECT_EQ(2, state.getParticleStrength(*center));
    state.addParticle(pos_at_border);
    EXPECT_EQ(3, state.getParticleStrength(*center));
    for (int x = pos_center.getX() - 4; x <= pos_center.getX() + 4; x++) {
        for (int y = pos_center.getY() - 4; y <= pos_center.getY() + 4; y++) {
            if (state.getParticleAt(IntVector(x, y)) != nullptr) {
                continue;
            }
            state.addParticle(IntVector(x, y));
        }
    }
    EXPECT_EQ((4 + 4 + 1) * (4 + 4 + 1), state.getParticleStrength(*center));
}

}
}
