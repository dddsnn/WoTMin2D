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
using B = NiceMockBlobState;

class BlobTest : public ::testing::Test {
    protected:
    BlobTest() :
        state(std::make_shared<NiceMockBlobState>()),
        td(),
        time_delta(std::chrono::milliseconds(73)) {
        // Return the test data particles by default.
        ON_CALL(*state, getParticles()).WillByDefault(ReturnRef(td.particles));
    }
    std::shared_ptr<NiceMockBlobState> state;
    TestData<P> td;
    std::chrono::milliseconds time_delta;
};

TEST_F(BlobTest, normalConstructorDoesntAddParticles) {
    EXPECT_CALL(*state, addParticle(_)).Times(0);
    Blob<P, B> blob(0, state);
}

TEST_F(BlobTest, circleConstructorAddsParticles) {
    IntVector center(7, 11);
    FloatVector center_float = static_cast<FloatVector>(center);
    float radius = 3.0f;
    int radius_ceil = static_cast<int>(std::ceil(radius) + 1);
    for (int x = center.getX() - radius_ceil; x < center.getX() + radius_ceil;
         x++)
    {
        for (int y = center.getY() - radius_ceil;
             y < center.getY() + radius_ceil; y++)
        {
            IntVector position(x, y);
            FloatVector position_float = static_cast<FloatVector>(position);
            if ((center_float - position_float).norm() <= radius) {
                EXPECT_CALL(*state, addParticle(position)).Times(1);
            } else {
                EXPECT_CALL(*state, addParticle(position)).Times(0);
            }
        }
    }
    Blob<P, B> blob(0, center, radius, td.width, td.height,
                                 state);
}

TEST_F(BlobTest, circleConstructorDoesntAddParticlesWithNegativeCoordinates) {
    IntVector center(1, 1);
    float radius = 3.0f;
    int radius_ceil = static_cast<int>(std::ceil(radius) + 1);
    for (int x = center.getX() - radius_ceil; x < center.getX() + radius_ceil;
         x++)
    {
        for (int y = center.getY() - radius_ceil;
             y < center.getY() + radius_ceil; y++)
        {
            IntVector position(x, y);
            if (x < 0 || y < 0) {
                EXPECT_CALL(*state, addParticle(position)).Times(0);
            } else {
                EXPECT_CALL(*state, addParticle(position)).Times(AnyNumber());
            }
        }
    }
    Blob<P, B> blob(0, center, radius, td.width, td.height,
                                 state);
}

TEST_F(BlobTest, circleConstructorDoesntAddParticlesOutOfBounds) {
    IntVector center(td.width - 2, td.height - 2);
    float radius = 3.0f;
    int radius_ceil = static_cast<int>(std::ceil(radius) + 1);
    int max_x = td.width - 1;
    int max_y = td.height - 1;
    for (int x = center.getX() - radius_ceil; x < center.getX() + radius_ceil;
         x++)
    {
        for (int y = center.getY() - radius_ceil;
             y < center.getY() + radius_ceil; y++)
        {
            IntVector position(x, y);
            if (x > max_x || y > max_y) {
                EXPECT_CALL(*state, addParticle(position)).Times(0);
            } else {
                EXPECT_CALL(*state, addParticle(position)).Times(AnyNumber());
            }
        }
    }
    Blob<P, B> blob(0, center, radius, td.width, td.height,
                                 state);
}

TEST_F(BlobTest, circleConstructorDoesntAddParticlesIfCenterIsOutOfBounds) {
    IntVector center(td.width + 4, td.height + 4);
    float radius = 3.0f;
    EXPECT_CALL(*state, addParticle(_)).Times(0);
    Blob<P, B> blob(0, center, radius, td.width, td.height, state);
}

TEST_F(BlobTest, advancesParticles) {
    EXPECT_CALL(*state, advanceParticles(time_delta)).Times(1);
    Blob<P, B> blob(0, state);
    blob.advanceParticles(time_delta);
}

TEST_F(BlobTest, onlyCollidesParticlesThatAreBlocked) {
    td.makeParticles({ td.lineA }, {});
    P* blockedParticle = td.particle_map[IntVector(3, 9)];
    P* blockingParticle = td.particle_map[IntVector(3, 10)];
    EXPECT_CALL(*state, collideParticles(Ref(*blockedParticle),
                                         Ref(*blockingParticle),
                                         Direction::north())).Times(1);
    for (const P* p: td.particles) {
        EXPECT_CALL(*state, moveParticle(Ref(*p), _)).Times(0);
    }
    Blob<P, B> blob(0, state);
    blob.handleParticle(*blockedParticle, Direction::north());
}

TEST_F(BlobTest, movesParticles) {
    td.makeParticles({}, { td.inside });
    P* particle = td.particle_map[td.inside];
    EXPECT_CALL(*state, moveParticle(Ref(*particle), Direction::north()))
        .Times(1);
    EXPECT_CALL(*state, collideParticles(_, _, _)).Times(0);
    Blob<P, B> blob(0, state);
    blob.handleParticle(*particle, Direction::north());
}

TEST_F(BlobTest, addsSingleNeighborAsFollowerIfParticleGetsDisconnected) {
    td.makeParticles({ td.lineA }, {});
    P* firstParticle = td.particle_map[IntVector(3, 10)];
    P* secondParticle = td.particle_map[IntVector(3, 9)];
    ON_CALL(*firstParticle, hasNeighbor())
        .WillByDefault(Return(false));
    for (int i = 4; i <= 9; i++) {
        P* particle = td.particle_map[IntVector(3, i)];
        EXPECT_CALL(*state, moveParticle(Ref(*particle), _)).Times(0);
    }
    EXPECT_CALL(*state, moveParticle(Ref(*firstParticle), Direction::north()))
        .Times(1);
    EXPECT_CALL(*state, addParticleFollowers(Ref(*firstParticle),
                                             ElementsAre(secondParticle)))
        .Times(1);
    Blob<P, B> blob(0, state);
    blob.handleParticle(*firstParticle, Direction::north());
}

TEST_F(BlobTest, addsThreeNeighborsAsFollowersIfParticleGetsDisconnected) {
    td.makeParticles({ td.block }, {});
    P* firstParticle = td.particle_map[IntVector(5, 4)];
    auto neighbors = { td.particle_map[IntVector(4, 4)],
                       td.particle_map[IntVector(5, 3)],
                       td.particle_map[IntVector(6, 4)] };
    ON_CALL(*firstParticle, hasNeighbor())
        .WillByDefault(Return(false));
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
    Blob<P, B> blob(0, state);
    blob.handleParticle(*firstParticle, Direction::north());
}

TEST_F(BlobTest, doesnAddFollowersIfParticleDoesntGetDisconnected) {
    td.makeParticles({ td.loop }, {});
    P* firstParticle = td.particle_map[IntVector(11, 11)];
    for (P* particle: td.particles) {
        if (particle != firstParticle) {
            EXPECT_CALL(*state, moveParticle(Ref(*particle), _)).Times(0);
        }
    }
    EXPECT_CALL(*state, moveParticle(Ref(*firstParticle), Direction::north()))
        .Times(1);
    EXPECT_CALL(*state, addParticleFollowers(_, _)).Times(0);
    Blob<P, B> blob(0, state);
    blob.handleParticle(*firstParticle, Direction::north());
}

}
}
