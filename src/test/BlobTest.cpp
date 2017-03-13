#include "mock/MockBlobState.hpp"
#include "mock/MockParticle.hpp"
#include "../game/BlobState.hpp"
#include "../game/Blob.hpp"
#include "../game/Vector.hpp"

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
using ::testing::_;
using ::testing::ReturnRef;
using ::testing::Return;
using ::testing::Ref;
using ::testing::AnyNumber;
using ::testing::Ne;
using ::testing::InSequence;

using ParticlePtr = std::shared_ptr<NiceMockParticle>;

/**
 * The test particle positions look as follows:
 *     012345678901234567890
 *    -----------------------
 *  0 |                     |
 *  1 |                     |
 *  2 |                     |
 *  3 |                     |
 *  4 |                     |
 *  5 |                     |
 *  6 |                     |
 *  7 |          lll        |
 *  8 |          l l        |
 *  9 |          lll        |
 * 10 |   abbbbbbb          |
 * 11 |   a                 |
 * 12 |   a                 |
 * 13 |   a                 |
 * 14 |   a                 |
 * 15 |   a           z     |
 * 16 |   akkkk             |
 * 17 |    kkkk             |
 * 18 |    kkkk             |
 * 19 |    kkkk             |
 * 20 |x         y          |
 *    -----------------------
 * x: inSouthWestCorner
 * y: onSouthBorder
 * z: inside
 * a: lineA
 * b: lineB
 * k: block
 * l: loop
 *
 * Particles can be made available in the particles and particle_map members by
 * calling makeParticles() with positions.
 */
class BlobTest : public ::testing::Test {
    protected:
    BlobTest() :
        state(std::make_shared<NiceMockBlobState>()),
        particles(),
        inSouthWestCorner(0, 0),
        onSouthBorder(10, 0),
        inside(15, 5),
        lineA(),
        lineB(),
        block(),
        loop() {
        // lineA: from (3, 4) to (3, 10)
        for (int i = 4; i <= 10; i++) {
            lineA.emplace_back(IntVector(3, i));
        }
        // lineB: from (4, 10) to (10, 10)
        for (int i = 4; i <= 10; i++) {
            lineB.emplace_back(IntVector(i, 10));
        }
        // block: square from (4, 1) to (7, 4)
        for (int i = 4; i <= 7; i++) {
            for (int j = 1; j <= 4; j++) {
                block.emplace_back(IntVector(i, j));
            }
        }
        // loop: square from (10, 11) to (12, 13) with middle particle missing
        for (int i = 10; i <= 12; i++) {
            for (int j = 11; j <= 13; j++) {
                if (i == 11 && j == 12) {
                    continue;
                }
                loop.emplace_back(IntVector(i, j));
            }
        }
        // Return the fixtures particles by default.
        ON_CALL(*state, getParticles()).WillByDefault(ReturnRef(particles));
    }
    virtual void
    makeParticles(std::initializer_list<std::vector<IntVector>> vectors,
                  std::initializer_list<IntVector> singles)
    {
        particles.clear();
        particle_map.clear();
        for (const auto& v: vectors) {
            for (const auto& p: v) {
                particle_map.emplace(p, std::make_shared<NiceMockParticle>(p));
            }
        }
        for (const auto& p: singles) {
            particle_map.emplace(p, std::make_shared<NiceMockParticle>(p));
        }
        auto getSecond = [] (std::pair<const IntVector, ParticlePtr>& v) {
                           return v.second;
                         };
        std::transform(particle_map.begin(), particle_map.end(),
                       std::back_inserter(particles), getSecond);
        // Set neighbors.
        for (const auto& p: particles) {
            for (auto d: Direction::all()) {
                auto i = particle_map.find(p->getPosition() + d.vector());
                if (i != particle_map.end()) {
                    p->setNeighbor(NiceMockParticle::MoveKey(), d, i->second);
                }
            }
        }
    }
    void expectConnected() {
        std::unordered_set<ParticlePtr> reachable;
        std::deque<ParticlePtr> queue;
        queue.push_back(particles[0]);
        reachable.insert(particles[0]);
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
        EXPECT_EQ(particles.size(), reachable.size());
    }
    std::shared_ptr<NiceMockBlobState> state;
    std::vector<ParticlePtr> particles;
    std::unordered_map<IntVector, ParticlePtr, IntVector::Hash> particle_map;
    IntVector inSouthWestCorner;
    IntVector onSouthBorder;
    IntVector inside;
    std::vector<IntVector> lineA;
    std::vector<IntVector> lineB;
    std::vector<IntVector> block;
    std::vector<IntVector> loop;
    static const unsigned int width = 21;
    static const unsigned int height = 21;
};

TEST_F(BlobTest, normalConstructorDoesntAddParticles) {
    EXPECT_CALL(*state, addParticle(_)).Times(0);
    Blob<NiceMockBlobState> blob(width, height, state);
}

// TODO Test with the exact number and position of particles.
TEST_F(BlobTest, circleConstructorAddsParticles) {
    EXPECT_CALL(*state, addParticle(_)).Times(AtLeast(1));
    Blob<NiceMockBlobState> blob(IntVector(10, 20), 3.0f, width, height, state);
}

TEST_F(BlobTest, advancesEachParticleOnce) {
    makeParticles({ lineA, lineB, block, loop },
                  { inSouthWestCorner, onSouthBorder, inside});
    for (const auto& p: particles) {
        EXPECT_CALL(*p, advance()).Times(1);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
}

TEST_F(BlobTest, advancesEachParticleBeforeMoving) {
    makeParticles({ lineA }, {});
    ParticlePtr firstParticle = particle_map[IntVector(3, 10)];
    ON_CALL(*firstParticle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::north(), true)));
    for (const auto& p: particles) {
        InSequence dummy;
        EXPECT_CALL(*p, advance()).Times(1);
        EXPECT_CALL(*state, moveParticle(_, _)).Times(AnyNumber());
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
}

TEST_F(BlobTest, onlyCollidesParticlesThatAreBlocked) {
    makeParticles({ lineA }, {});
    ParticlePtr blockedParticle = particle_map[IntVector(3, 9)];
    ParticlePtr blockingParticle = particle_map[IntVector(3, 10)];
    ON_CALL(*blockedParticle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::north(), true)));
    EXPECT_CALL(*blockedParticle, collideWith(Ref(*blockingParticle))).Times(1);
    for (const auto& p: particles) {
        EXPECT_CALL(*state, moveParticle(p, _)).Times(0);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
}

TEST_F(BlobTest, movesSingleParticles) {
    makeParticles({}, { inside });
    ParticlePtr particle = particle_map[inside];
    ON_CALL(*particle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::north(), true)));
    EXPECT_CALL(*state, moveParticle(particle, Direction::north())).Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
}

TEST_F(BlobTest, movesParticlesIndependently) {
    makeParticles({}, { inSouthWestCorner, onSouthBorder, inside });
    ParticlePtr swc = particle_map[inSouthWestCorner];
    ParticlePtr sb = particle_map[onSouthBorder];
    ParticlePtr in = particle_map[inside];
    ON_CALL(*swc, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::north(), true)));
    ON_CALL(*sb, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::east(), true)));
    EXPECT_CALL(*state, moveParticle(swc, Direction::north()))
        .Times(1);
    EXPECT_CALL(*state, moveParticle(sb, Direction::east()))
        .Times(1);
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
}

TEST_F(BlobTest, movesEntireLineOfParticlesIfOnlyTheFirstWantsToMove) {
    makeParticles({ lineA }, {});
    ParticlePtr firstParticle = particle_map[IntVector(3, 10)];
    ON_CALL(*firstParticle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::north(), true)));
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = particle_map[IntVector(3, i)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::north()))
            .Times(1);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
}

TEST_F(BlobTest, movesEntireLineOfParticlesOnlyOnceIfAllWantToMove) {
    makeParticles({ lineA }, {});
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = particle_map[IntVector(3, i)];
        ON_CALL(*particle, getMovement())
            .WillByDefault(Return(std::make_pair(Direction::north(), true)));
        EXPECT_CALL(*state, moveParticle(particle, Direction::north()))
            .Times(1);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
}

TEST_F(BlobTest, movesEntireLineOnEveryAdvance) {
    makeParticles({ lineA }, {});
    ParticlePtr firstParticle = particle_map[IntVector(3, 10)];
    ON_CALL(*firstParticle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::north(), true)));
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = particle_map[IntVector(3, i)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::north()))
            .Times(3);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
    blob.advance();
    blob.advance();
}

TEST_F(BlobTest, movesOnlyLineOfBlockIfConnectivityStays) {
    makeParticles({ block }, {});
    ParticlePtr firstParticle = particle_map[IntVector(6, 4)];
    ON_CALL(*firstParticle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::north(), true)));
    for (int i = 4; i <= 7; i++) {
        for (int j = 1; j <= 4; j++) {
            ParticlePtr particle = particle_map[IntVector(i, j)];
            if (i == 6) {
                // Particles in line must move.
                EXPECT_CALL(*state, moveParticle(particle, Direction::north()))
                    .Times(1);
            } else {
                // All others must stay.
                EXPECT_CALL(*state, moveParticle(particle, _)).Times(0);
            }
        }
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
}

TEST_F(BlobTest, dragsLineBehindToMaintainConnectivity) {
    makeParticles({ lineA, lineB }, {});
    ParticlePtr firstParticle = particle_map[IntVector(3, 4)];
    ON_CALL(*firstParticle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::south(), true)));
    EXPECT_CALL(*state, moveParticle(_, _)).Times(AnyNumber());
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = particle_map[IntVector(i, 10)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::west()))
            .Times(1);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
}

TEST_F(BlobTest, dragsBlockBehindToMaintainConnectivity) {
    makeParticles({ lineA, block }, {});
    ParticlePtr firstParticle = particle_map[IntVector(3, 10)];
    ON_CALL(*firstParticle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::north(), true)));
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
    expectConnected();
}

TEST_F(BlobTest, movesLineOfLoopAndMaintainsConnectivity) {
    makeParticles({ loop }, {});
    ParticlePtr firstParticle = particle_map[IntVector(12, 13)];
    ON_CALL(*firstParticle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::east(), true)));
    EXPECT_CALL(*state, moveParticle(_, _)).Times(AnyNumber());
    for (int i = 10; i <= 12; i++) {
        ParticlePtr particle = particle_map[IntVector(i, 13)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::east())).Times(1);
        EXPECT_CALL(*state, moveParticle(particle, Ne(Direction::east())))
            .Times(0);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
    expectConnected();
}

TEST_F(BlobTest, dragsLoopBehindAndMaintainsConnectivity) {
    makeParticles({ lineB, loop }, {});
    ParticlePtr firstParticle = particle_map[IntVector(4, 10)];
    ON_CALL(*firstParticle, getMovement())
        .WillByDefault(Return(std::make_pair(Direction::west(), true)));
    EXPECT_CALL(*state, moveParticle(_, _)).Times(AnyNumber());
    for (int i = 4; i <= 10; i++) {
        ParticlePtr particle = particle_map[IntVector(i, 10)];
        EXPECT_CALL(*state, moveParticle(particle, Direction::west())).Times(1);
        EXPECT_CALL(*state, moveParticle(particle, Ne(Direction::west())))
            .Times(0);
    }
    Blob<NiceMockBlobState, NiceMockParticle> blob(width, height, state);
    blob.advance();
    expectConnected();
}

}
}
