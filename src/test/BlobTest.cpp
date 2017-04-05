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
#include <chrono>

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

using P = NiceMockParticle;

class BlobTest : public ::testing::Test {
    protected:
    BlobTest() :
        state(std::make_shared<NiceMockBlobState>()),
        td(),
        time_delta(std::chrono::milliseconds(73)) {
        // Return the test data particles by default.
        ON_CALL(*state, getParticles()).WillByDefault(ReturnRef(td.particles));
    }
    void expectConnected() {
        std::unordered_set<P*> reachable;
        std::deque<P*> queue;
        queue.push_back(td.particles[0]);
        reachable.insert(td.particles[0]);
        while (!queue.empty()) {
            P* particle = queue.front();
            queue.pop_front();
            for (Direction direction: Direction::all()) {
                P* neighbor = particle->getNeighbor(direction);
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
    std::chrono::milliseconds time_delta;
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
    ON_CALL(*state, getHighestMobilityParticle())
        .WillByDefault(Return(td.particles.front()));
    EXPECT_CALL(*state, advanceParticles(time_delta)).Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

TEST_F(BlobTest, advancesParticlesBeforeMoving) {
    td.makeParticles({ td.lineA }, {});
    ON_CALL(*state, getHighestMobilityParticle())
        .WillByDefault(Return(td.particles.front()));
    P* firstParticle = td.particle_map[IntVector(3, 10)];
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*firstParticle, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(
            static_cast<FloatVector>(Direction::north().vector())))
        .WillRepeatedly(DoDefault());
    {
        InSequence dummy;
        EXPECT_CALL(*state, advanceParticles(time_delta)).Times(1);
        EXPECT_CALL(*state, moveParticle(_, _)).Times(AnyNumber());
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

TEST_F(BlobTest, onlyCollidesParticlesThatAreBlocked) {
    td.makeParticles({ td.lineA }, {});
    P* blockedParticle = td.particle_map[IntVector(3, 9)];
    P* blockingParticle = td.particle_map[IntVector(3, 10)];
    EXPECT_CALL(*state, getHighestMobilityParticle())
        .Times(AnyNumber())
        .WillOnce(Return(blockedParticle))
        .WillOnce(Return(blockingParticle))
        .WillRepeatedly(Return(td.particles.front()));
    ON_CALL(*blockedParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    ON_CALL(*blockedParticle, canMove())
        .WillByDefault(Return(true));
    EXPECT_CALL(*state, collideParticles(Ref(*blockedParticle),
                                         Ref(*blockingParticle),
                                         Direction::north())).Times(1);
    for (const P* p: td.particles) {
        EXPECT_CALL(*state, moveParticle(Ref(*p), _)).Times(0);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

TEST_F(BlobTest, quitsWhenHighestMobilityParticleCantMove) {
    td.makeParticles({ td.lineA }, {});
    for (P* particle: td.particles) {
        EXPECT_CALL(*state, getHighestMobilityParticle())
            .Times(AtMost(1))
            .WillOnce(Return(particle));
    }
    EXPECT_CALL(*state, collideParticles(_, _, _)).Times(0);
    EXPECT_CALL(*state, moveParticle(_, _)).Times(0);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

TEST_F(BlobTest, movesSingleParticles) {
    td.makeParticles({}, { td.inside });
    ON_CALL(*state, getHighestMobilityParticle())
        .WillByDefault(Return(td.particles.front()));
    P* particle = td.particle_map[td.inside];
    ON_CALL(*particle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*particle, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*state, moveParticle(Ref(*particle), Direction::north()))
        .Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

TEST_F(BlobTest, movesSingleParticlesMultipleTimes) {
    td.makeParticles({}, { td.inside });
    ON_CALL(*state, getHighestMobilityParticle())
        .WillByDefault(Return(td.particles.front()));
    P* particle = td.particle_map[td.inside];
    ON_CALL(*particle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*particle, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*state, moveParticle(Ref(*particle), Direction::north()))
        .Times(3);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

TEST_F(BlobTest, movesParticlesIndependently) {
    td.makeParticles({}, { td.inSouthWestCorner, td.onSouthBorder, td.inside });
    P* swc = td.particle_map[td.inSouthWestCorner];
    P* sb = td.particle_map[td.onSouthBorder];
    P* in = td.particle_map[td.inside];
    {
        InSequence dummy;
        // Return particles starting with the ones with the highest mobility so
        // the Blob doesn't quit early.
        for (P* particle: { swc, sb, in }) {
            EXPECT_CALL(*state, getHighestMobilityParticle())
                .WillOnce(Return(particle));
        }
    }
    ON_CALL(*swc, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*swc, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    ON_CALL(*sb, getPressureDirection())
        .WillByDefault(Return(Direction::east()));
    EXPECT_CALL(*sb, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    ON_CALL(*in, canMove()).WillByDefault(Return(false));
    EXPECT_CALL(*state, moveParticle(Ref(*swc), Direction::north()))
        .Times(1);
    EXPECT_CALL(*state, moveParticle(Ref(*sb), Direction::east()))
        .Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

TEST_F(BlobTest, addsSingleNeighborAsFollowerIfParticleGetsDisconnected) {
    td.makeParticles({ td.lineA }, {});
    P* firstParticle = td.particle_map[IntVector(3, 10)];
    P* secondParticle = td.particle_map[IntVector(3, 9)];
    ON_CALL(*state, getHighestMobilityParticle())
        .WillByDefault(Return(firstParticle));
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    ON_CALL(*firstParticle, hasNeighbor())
        .WillByDefault(Return(false));
    EXPECT_CALL(*firstParticle, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    for (int i = 4; i <= 9; i++) {
        P* particle = td.particle_map[IntVector(3, i)];
        EXPECT_CALL(*state, moveParticle(Ref(*particle), _)).Times(0);
    }
    EXPECT_CALL(*state, moveParticle(Ref(*firstParticle), Direction::north()))
        .Times(1);
    EXPECT_CALL(*state, addParticleFollowers(Ref(*firstParticle),
                                             ElementsAre(secondParticle)))
        .Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

TEST_F(BlobTest, addsThreeNeighborsAsFollowersIfParticleGetsDisconnected) {
    td.makeParticles({ td.block }, {});
    P* firstParticle = td.particle_map[IntVector(5, 4)];
    auto neighbors = { td.particle_map[IntVector(4, 4)],
                       td.particle_map[IntVector(5, 3)],
                       td.particle_map[IntVector(6, 4)] };
    ON_CALL(*state, getHighestMobilityParticle())
        .WillByDefault(Return(firstParticle));
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    ON_CALL(*firstParticle, hasNeighbor())
        .WillByDefault(Return(false));
    EXPECT_CALL(*firstParticle, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    for (P* particle: td.particles) {
        if (particle != firstParticle) {
            EXPECT_CALL(*state, moveParticle(Ref(*particle), _)).Times(0);
        }
    }
    EXPECT_CALL(*state, moveParticle(Ref(*firstParticle), Direction::north()))
        .Times(1);
    EXPECT_CALL(*state,
                addParticleFollowers(Ref(*firstParticle),
                                     UnorderedElementsAreArray(neighbors)))
        .Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

TEST_F(BlobTest, doesnAddFollowersIfParticleDoesntGetDisconnected) {
    td.makeParticles({ td.loop }, {});
    P* firstParticle = td.particle_map[IntVector(11, 11)];
    ON_CALL(*state, getHighestMobilityParticle())
        .WillByDefault(Return(firstParticle));
    ON_CALL(*firstParticle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*firstParticle, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    for (P* particle: td.particles) {
        if (particle != firstParticle) {
            EXPECT_CALL(*state, moveParticle(Ref(*particle), _)).Times(0);
        }
    }
    EXPECT_CALL(*state, moveParticle(Ref(*firstParticle), Direction::north()))
        .Times(1);
    EXPECT_CALL(*state, addParticleFollowers(_, _)).Times(0);
    Blob<NiceMockBlobState, NiceMockParticle> blob(td.width, td.height, state);
    blob.advance(time_delta);
}

}
}
