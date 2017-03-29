#include "mock/MockBlobState.hpp"
#include "mock/MockParticle.hpp"
#include "../game/BlobState.hpp"
#include "../game/Blob.hpp"
#include "../game/Vector.hpp"
#include "TestData.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <deque>

namespace wotmin2d {
namespace test {

using mock::NiceMockBlobState;
using mock::NiceMockParticle;

using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::_;
using ::testing::ReturnRef;
using ::testing::ReturnRefOfCopy;
using ::testing::Return;
using ::testing::Ref;
using ::testing::AnyNumber;
using ::testing::Ne;
using ::testing::InSequence;
using ::testing::DoDefault;

using ParticlePtr = std::shared_ptr<NiceMockParticle>;

class BlobTest : public ::testing::Test {
    protected:
    BlobTest() :
        state(std::make_shared<NiceMockBlobState>()),
        td() {
        // Return the test data particles by default.
        ON_CALL(*state, getParticles()).WillByDefault(ReturnRef(td.particles));
    }
    void expectConnected() {
        std::unordered_set<ParticlePtr> reachable;
        std::deque<ParticlePtr> queue;
        queue.push_back(td.particles[0]);
        reachable.insert(td.particles[0]);
        while (!queue.empty()) {
            ParticlePtr particle = queue.front();
            queue.pop_front();
            for (Direction direction: Direction::all()) {
                ParticlePtr neighbor = particle->getNeighbor(direction);
                if (neighbor == nullptr || reachable.count(neighbor) > 0) {
                    continue;
                }
                queue.push_back(neighbor);
                reachable.insert(neighbor);
            }
        }
        EXPECT_EQ(td.particles.size(), reachable.size());
    }
    std::shared_ptr<NiceMockBlobState> state;
    TestData td;
};

TEST_F(BlobTest, normalConstructorDoesntAddParticles) {
    EXPECT_CALL(*state, addParticle(_)).Times(0);
    Blob<NiceMockBlobState> blob(td.width, td.height, state);
}

// TODO Test with the exact number and position of particles.
TEST_F(BlobTest, circleConstructorAddsParticles) {
    EXPECT_CALL(*state, addParticle(_)).Times(AtLeast(1));
    Blob<NiceMockBlobState> blob(IntVector(10, 20), 3.0f, td.width, td.height,
                                 state);
}

TEST_F(BlobTest, advancesParticles) {
    td.makeParticles({ td.lineA, td.lineB, td.block, td.loop },
                     { td.inSouthWestCorner, td.onSouthBorder, td.inside});
    ON_CALL(*state, getHighestPressureParticle())
        .WillByDefault(ReturnRef(td.particles.front()));
    EXPECT_CALL(*state, advanceParticles()).Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, advancesParticlesBeforeMoving) {
    td.makeParticles({ td.lineA }, {});
    ON_CALL(*state, getHighestPressureParticle())
        .WillByDefault(ReturnRef(td.particles.front()));
    ParticlePtr firstParticle = td.particle_map[IntVector(3, 10)];
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*firstParticle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::north().vector())))
        .WillRepeatedly(DoDefault());
    {
        InSequence dummy;
        EXPECT_CALL(*state, advanceParticles()).Times(1);
        EXPECT_CALL(*state, moveParticle(_, _)).Times(AnyNumber());
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, onlyCollidesParticlesThatAreBlocked) {
    td.makeParticles({ td.lineA }, {});
    ParticlePtr blockedParticle = td.particle_map[IntVector(3, 9)];
    ParticlePtr blockingParticle = td.particle_map[IntVector(3, 10)];
    EXPECT_CALL(*state, getHighestPressureParticle())
        .Times(AnyNumber())
        .WillOnce(ReturnRef(blockedParticle))
        .WillOnce(ReturnRef(blockingParticle))
        .WillRepeatedly(ReturnRef(td.particles.front()));
    ON_CALL(*blockedParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*blockedParticle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::north().vector())))
        .WillRepeatedly(DoDefault());
    EXPECT_CALL(*state, collideParticles(blockedParticle,
                                         blockingParticle)).Times(1);
    for (const auto& p: td.particles) {
        EXPECT_CALL(*state, moveParticle(p, _)).Times(0);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, quitsWhenHighestPressureIsZero) {
    td.makeParticles({ td.lineA }, {});
    for (const auto& particle: td.particles) {
        EXPECT_CALL(*state, getHighestPressureParticle())
            .Times(AtMost(1))
            .WillOnce(ReturnRef(particle));
    }
    EXPECT_CALL(*state, collideParticles(_, _)).Times(0);
    EXPECT_CALL(*state, moveParticle(_, _)).Times(0);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, movesSingleParticles) {
    td.makeParticles({}, { td.inside });
    ON_CALL(*state, getHighestPressureParticle())
        .WillByDefault(ReturnRef(td.particles.front()));
    ParticlePtr particle = td.particle_map[td.inside];
    ON_CALL(*particle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*particle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::north().vector())))
        .WillRepeatedly(DoDefault());
    EXPECT_CALL(*state, moveParticle(particle, Direction::north())).Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, movesParticlesIndependently) {
    td.makeParticles({}, { td.inSouthWestCorner, td.onSouthBorder, td.inside });
    ParticlePtr swc = td.particle_map[td.inSouthWestCorner];
    ParticlePtr sb = td.particle_map[td.onSouthBorder];
    ParticlePtr in = td.particle_map[td.inside];
    {
        InSequence dummy;
        // Return particles starting with the ones with the highest pressure so
        // the Blob doesn't quit early.
        for (const auto& particle: { swc, sb, in }) {
            EXPECT_CALL(*state, getHighestPressureParticle())
                .WillOnce(ReturnRef(particle));
        }
    }
    ON_CALL(*swc, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*swc, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::north().vector())))
        .WillRepeatedly(DoDefault());
    ON_CALL(*sb, getPressureDirection())
        .WillByDefault(Return(Direction::east()));
    EXPECT_CALL(*sb, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::east().vector())))
        .WillRepeatedly(DoDefault());
    EXPECT_CALL(*state, moveParticle(swc, Direction::north()))
        .Times(1);
    EXPECT_CALL(*state, moveParticle(sb, Direction::east()))
        .Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, movesEntireLineOfParticlesIfOnlyTheFirstWantsToMove) {
    td.makeParticles({ td.lineA }, {});
    ON_CALL(*state, getHighestPressureParticle())
        .WillByDefault(ReturnRef(td.particles.front()));
    ParticlePtr firstParticle = td.particle_map[IntVector(3, 10)];
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*firstParticle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::north().vector())))
        .WillRepeatedly(DoDefault());
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = td.particle_map[IntVector(3, i)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::north()))
            .Times(1);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, movesEntireLineOfParticlesOnlyOnceIfAllWantToMove) {
    td.makeParticles({ td.lineA }, {});
    ON_CALL(*state, getHighestPressureParticle())
        .WillByDefault(ReturnRef(td.particles.front()));
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = td.particle_map[IntVector(3, i)];
        ON_CALL(*particle, getPressureDirection())
            .WillByDefault(Return(Direction::north()));
        EXPECT_CALL(*particle, getPressure())
            .Times(AnyNumber())
            .WillOnce(ReturnRefOfCopy(
                static_cast<FloatVector>(Direction::north().vector())))
            .WillRepeatedly(DoDefault());
        EXPECT_CALL(*state, moveParticle(particle, Direction::north()))
            .Times(1);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, movesEntireLineOnEveryAdvance) {
    td.makeParticles({ td.lineA }, {});
    ON_CALL(*state, getHighestPressureParticle())
        .WillByDefault(ReturnRef(td.particles.front()));
    ParticlePtr firstParticle = td.particle_map[IntVector(3, 10)];
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    {
        InSequence dummy;
        for (int i = 0; i < 3; i++) {
            // Alternate between wanting to north and not wanting to go anywhere
            // so the Blob moves the particle north once and then terminates the
            // step.
            EXPECT_CALL(*firstParticle, getPressure())
                .WillOnce(ReturnRefOfCopy(
                    static_cast<FloatVector>(Direction::north().vector())))
                .WillOnce(ReturnRefOfCopy(FloatVector(0.0f, 0.0f)));
        }
    }
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = td.particle_map[IntVector(3, i)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::north()))
            .Times(3);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
    blob.advance();
    blob.advance();
}

TEST_F(BlobTest, movesOnlyLineOfBlockIfConnectivityStays) {
    td.makeParticles({ td.block }, {});
    ParticlePtr firstParticle = td.particle_map[IntVector(6, 4)];
    EXPECT_CALL(*state, getHighestPressureParticle())
        .Times(AnyNumber())
        .WillRepeatedly(ReturnRef(firstParticle));
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*firstParticle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::north().vector())))
        .WillRepeatedly(DoDefault());
    for (int i = 4; i <= 7; i++) {
        for (int j = 1; j <= 4; j++) {
            ParticlePtr particle = td.particle_map[IntVector(i, j)];
            if (i == 6) {
                // td.particles in line must move.
                EXPECT_CALL(*state, moveParticle(particle, Direction::north()))
                    .Times(1);
            } else {
                // All others must stay.
                EXPECT_CALL(*state, moveParticle(particle, _)).Times(0);
            }
        }
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, dragsLineBehindToMaintainConnectivity) {
    td.makeParticles({ td.lineA, td.lineB }, {});
    ParticlePtr firstParticle = td.particle_map[IntVector(3, 4)];
    EXPECT_CALL(*state, getHighestPressureParticle())
        .Times(AnyNumber())
        .WillRepeatedly(ReturnRef(firstParticle));
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::south()));
    EXPECT_CALL(*firstParticle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::south().vector())))
        .WillRepeatedly(DoDefault());
    EXPECT_CALL(*state, moveParticle(_, _)).Times(AnyNumber());
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = td.particle_map[IntVector(i, 10)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::west()))
            .Times(1);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
}

TEST_F(BlobTest, dragsBlockBehindToMaintainConnectivity) {
    td.makeParticles({ td.lineA, td.block }, {});
    ON_CALL(*state, getHighestPressureParticle())
        .WillByDefault(ReturnRef(td.particles.front()));
    ParticlePtr firstParticle = td.particle_map[IntVector(3, 10)];
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*firstParticle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::north().vector())))
        .WillRepeatedly(DoDefault());
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
    expectConnected();
}

TEST_F(BlobTest, movesLineOfLoopAndMaintainsConnectivity) {
    td.makeParticles({ td.loop }, {});
    ON_CALL(*state, getHighestPressureParticle())
        .WillByDefault(ReturnRef(td.particles.front()));
    ParticlePtr firstParticle = td.particle_map[IntVector(12, 13)];
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::east()));
    EXPECT_CALL(*firstParticle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::east().vector())))
        .WillRepeatedly(DoDefault());
    EXPECT_CALL(*state, moveParticle(_, _)).Times(AnyNumber());
    for (int i = 10; i <= 12; i++) {
        ParticlePtr particle = td.particle_map[IntVector(i, 13)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::east())).Times(1);
        EXPECT_CALL(*state, moveParticle(particle, Ne(Direction::east())))
            .Times(0);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
    expectConnected();
}

TEST_F(BlobTest, dragsLoopBehindAndMaintainsConnectivity) {
    td.makeParticles({ td.lineB, td.loop }, {});
    ParticlePtr firstParticle = td.particle_map[IntVector(4, 10)];
    ON_CALL(*state, getHighestPressureParticle())
        .WillByDefault(ReturnRef(firstParticle));
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::west()));
    EXPECT_CALL(*firstParticle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::west().vector())))
        .WillRepeatedly(DoDefault());
    EXPECT_CALL(*state, moveParticle(_, _)).Times(AnyNumber());
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = td.particle_map[IntVector(i, 10)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::west())).Times(1);
        EXPECT_CALL(*state, moveParticle(particle, Ne(Direction::west())))
            .Times(0);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance();
    expectConnected();
}

}
}
