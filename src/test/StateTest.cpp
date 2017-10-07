#include "TestData.hpp"
#include "../game/State.hpp"
#include "mock/MockBlob.hpp"
#include "mock/MockParticle.hpp"
#include "../game/Vector.hpp"

#include <gtest/gtest.h>
#include <chrono>

namespace wotmin2d {
namespace test {

using mock::NiceMockParticle;
using mock::NiceMockBlob;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRefOfCopy;
using ::testing::Ref;
using ::testing::AnyNumber;
using ::testing::AtMost;
using ::testing::InSequence;
using ::testing::WithArg;

using P = NiceMockParticle;
using B = NiceMockBlob;

ACTION_P2(KillPressureInDirection, particle, direction) {
    particle->killPressureInDirection({}, direction);
}

ACTION_P2(MoveParticle, particle, direction) {
    particle->move({}, direction);
}

class StateTest : public ::testing::Test {
    protected:
    StateTest() :
        td(),
        width(td.width),
        height(td.height),
        state(width, height),
        time_delta(std::chrono::milliseconds(73)) {}
    TestData<P> td;
    const unsigned int width;
    const unsigned int height;
    State<P, B> state;
    std::chrono::milliseconds time_delta;
};

TEST_F(StateTest, advancesParticles) {
    td.makeParticles({ td.lineA, td.lineB, td.block, td.loop },
                     { td.inSouthWestCorner, td.onSouthBorder, td.inside });
    state.emplaceBlob(0, IntVector(0, 0), 2);
    state.emplaceBlob(7, IntVector(5, 8), 4);
    B& blob1 = const_cast<B&>(state.getBlobs().at(0));
    B& blob2 = const_cast<B&>(state.getBlobs().at(7));
    ON_CALL(blob1, getHighestMobilityParticle())
        .WillByDefault(Return(td.particles[0]));
    ON_CALL(blob2, getHighestMobilityParticle())
        .WillByDefault(Return(td.particles[1]));
    EXPECT_CALL(blob1, advanceParticles(time_delta)).Times(1);
    EXPECT_CALL(blob2, advanceParticles(time_delta)).Times(1);
    state.advance(time_delta);
}

TEST_F(StateTest, advancesParticlesBeforeMoving) {
    td.makeParticles({ td.lineA }, {});
    state.emplaceBlob(0, IntVector(0, 0), 2);
    B& blob = const_cast<B&>(state.getBlobs().at(0));
    ON_CALL(blob, getHighestMobilityParticle())
        .WillByDefault(Return(td.particles[0]));
    {
        InSequence dummy;
        EXPECT_CALL(blob, advanceParticles(time_delta)).Times(1);
        EXPECT_CALL(blob, handleParticle(_, _)).Times(AnyNumber());
        EXPECT_CALL(blob, collideParticleWithWall(_, _)).Times(AnyNumber());
    }
    state.advance(time_delta);
}

TEST_F(StateTest, letsBlobMoveSingleParticles) {
    td.makeParticles({}, { td.inSouthWestCorner });
    state.emplaceBlob(0, IntVector(0, 0), 2);
    B& blob = const_cast<B&>(state.getBlobs().at(0));
    P* particle = td.particles[0];
    ON_CALL(blob, getHighestMobilityParticle())
        .WillByDefault(Return(particle));
    ON_CALL(*particle, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*particle, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(blob, handleParticle(Ref(*particle), Direction::north()))
        .Times(1);
    EXPECT_CALL(blob, collideParticleWithWall(_, _)).Times(0);
    state.advance(time_delta);
}

TEST_F(StateTest, letsBlobMoveSingleParticlesMultipleTimes) {
    td.makeParticles({}, { td.inSouthWestCorner });
    state.emplaceBlob(0, IntVector(0, 0), 2);
    B& blob = const_cast<B&>(state.getBlobs().at(0));
    P* particle = td.particles[0];
    ON_CALL(blob, getHighestMobilityParticle())
        .WillByDefault(Return(particle));
    EXPECT_CALL(*particle, getPressureDirection())
        .WillOnce(Return(Direction::north()))
        .WillOnce(Return(Direction::east()))
        .WillRepeatedly(Return(Direction::north()));
    EXPECT_CALL(*particle, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    {
        InSequence dummy;
        EXPECT_CALL(blob, handleParticle(Ref(*particle), Direction::north()))
            .Times(1);
        EXPECT_CALL(blob, handleParticle(Ref(*particle), Direction::east()))
            .Times(1);
        EXPECT_CALL(blob, handleParticle(Ref(*particle), Direction::north()))
            .Times(1);
    }
    EXPECT_CALL(blob, collideParticleWithWall(_, _)).Times(0);
    state.advance(time_delta);
}

TEST_F(StateTest, letsBlobMoveParticlesIndependently) {
    td.makeParticles({}, { td.inSouthWestCorner, td.onSouthBorder });
    state.emplaceBlob(0, IntVector(0, 0), 2);
    B& blob = const_cast<B&>(state.getBlobs().at(0));
    P* swc = td.particle_map[td.inSouthWestCorner];
    P* osb = td.particle_map[td.onSouthBorder];
    EXPECT_CALL(blob, getHighestMobilityParticle())
        .WillOnce(Return(swc))
        .WillOnce(Return(osb))
        .WillOnce(Return(swc))
        .WillRepeatedly(Return(osb));
    EXPECT_CALL(*swc, getPressureDirection())
        .WillOnce(Return(Direction::north()))
        .WillRepeatedly(Return(Direction::east()));
    EXPECT_CALL(*swc, canMove())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*osb, getPressureDirection())
        .WillOnce(Return(Direction::west()))
        .WillRepeatedly(Return(Direction::north()));
    EXPECT_CALL(*osb, canMove())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    {
        InSequence dummy;
        EXPECT_CALL(blob, handleParticle(Ref(*swc), Direction::north()))
            .Times(1);
        EXPECT_CALL(blob, handleParticle(Ref(*osb), Direction::west()))
            .Times(1);
        EXPECT_CALL(blob, handleParticle(Ref(*swc), Direction::east()))
            .Times(1);
        EXPECT_CALL(blob, handleParticle(Ref(*osb), Direction::north()))
            .Times(1);
    }
    EXPECT_CALL(blob, collideParticleWithWall(_, _)).Times(0);
    state.advance(time_delta);
}

TEST_F(StateTest, quitsWhenHighestMobilityParticleCantMove) {
    td.makeParticles({ td.lineA }, {});
    state.emplaceBlob(0, IntVector(0, 0), 2);
    B& blob = const_cast<B&>(state.getBlobs().at(0));
    ON_CALL(blob, getHighestMobilityParticle())
        .WillByDefault(Return(td.particles[0]));
    for (P* particle: td.particles) {
        EXPECT_CALL(blob, getHighestMobilityParticle())
            .Times(AtMost(1))
            .WillOnce(Return(particle));
    }
    EXPECT_CALL(blob, handleParticle(_, _)).Times(0);
    EXPECT_CALL(blob, collideParticleWithWall(_, _)).Times(0);
    state.advance(time_delta);
}

TEST_F(StateTest, doesntMoveParticlesBeyondBounds) {
    td.makeParticles({}, { td.inSouthWestCorner, td.inNorthEastCorner });
    state.emplaceBlob(0, IntVector(0, 0), 2);
    B& blob = const_cast<B&>(state.getBlobs().at(0));
    P* swc = td.particle_map[td.inSouthWestCorner];
    P* nec = td.particle_map[td.inNorthEastCorner];
    EXPECT_CALL(blob, getHighestMobilityParticle())
        .Times(AnyNumber())
        .WillOnce(Return(swc))
        .WillOnce(Return(swc))
        .WillOnce(Return(nec))
        .WillOnce(Return(nec))
        .WillRepeatedly(Return(td.particles.front()));
    EXPECT_CALL(*swc, getPressureDirection())
        .Times(AnyNumber())
        .WillOnce(Return(Direction::west()))
        .WillRepeatedly(Return(Direction::south()));
    EXPECT_CALL(*nec, getPressureDirection())
        .Times(AnyNumber())
        .WillOnce(Return(Direction::east()))
        .WillRepeatedly(Return(Direction::north()));
    EXPECT_CALL(*swc, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*nec, canMove())
        .Times(AnyNumber())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(blob, collideParticleWithWall(Ref(*swc), Direction::west()))
        .Times(1);
    EXPECT_CALL(blob, collideParticleWithWall(Ref(*swc), Direction::south()))
        .Times(1);
    EXPECT_CALL(blob, collideParticleWithWall(Ref(*nec), Direction::east()))
        .Times(1);
    EXPECT_CALL(blob, collideParticleWithWall(Ref(*nec), Direction::north()))
        .Times(1);
    EXPECT_CALL(blob, handleParticle(_, _)).Times(0);
    state.advance(time_delta);
}

TEST_F(StateTest, usesHighestMobilityBlobFirst) {
    td.makeParticles({}, { td.inSouthWestCorner, td.onSouthBorder });
    state.emplaceBlob(0, IntVector(0, 0), 2);
    state.emplaceBlob(7, IntVector(5, 8), 4);
    B& blob1 = const_cast<B&>(state.getBlobs().at(0));
    B& blob2 = const_cast<B&>(state.getBlobs().at(7));
    P* p1 = td.particle_map[td.inSouthWestCorner];
    P* p2 = td.particle_map[td.onSouthBorder];
    ON_CALL(blob1, getHighestMobilityParticle())
        .WillByDefault(Return(p1));
    ON_CALL(blob2, getHighestMobilityParticle())
        .WillByDefault(Return(p2));
    p1->setTarget(td.inSouthWestCorner + Direction::east().vector(), 1.5f);
    p2->setTarget(td.onSouthBorder + Direction::north().vector(), 2.0f);
    p1->advance({}, std::chrono::milliseconds(1000));
    p2->advance({}, std::chrono::milliseconds(1000));
    {
        InSequence dummy;
        EXPECT_CALL(blob2, handleParticle(Ref(*p2), Direction::north()))
            .WillOnce(MoveParticle(p2, Direction::north()));
        EXPECT_CALL(blob1, handleParticle(Ref(*p1), Direction::east()))
            .WillOnce(MoveParticle(p1, Direction::east()));
        EXPECT_CALL(blob2, handleParticle(Ref(*p2), Direction::north()))
            .WillOnce(MoveParticle(p2, Direction::north()));
    }
    state.advance(time_delta);
}

TEST_F(StateTest, doesntMoveParticleIntoHostileParticle) {
    td.makeParticles({ td.lineA }, {});
    TestData<P> td2;
    td2.makeParticles({ td2.lineB }, {});
    state.emplaceBlob(0, IntVector(0, 0), 2);
    state.emplaceBlob(7, IntVector(5, 8), 4);
    B& blob_right = const_cast<B&>(state.getBlobs().at(0));
    B& blob_left = const_cast<B&>(state.getBlobs().at(7));
    P* right = td2.particle_map[IntVector(4, 10)];
    P* left = td.particle_map[IntVector(3, 10)];
    ON_CALL(blob_right, getHighestMobilityParticle())
        .WillByDefault(Return(right));
    ON_CALL(blob_left, getHighestMobilityParticle())
        .WillByDefault(Return(left));
    ON_CALL(blob_right, getParticleAt(IntVector(4, 10)))
        .WillByDefault(Return(right));
    ON_CALL(blob_left, getParticleAt(IntVector(3, 10)))
        .WillByDefault(Return(left));
    left->setTarget(IntVector(5, 10), 1.0f);
    left->advance({}, std::chrono::milliseconds(1000));
    EXPECT_CALL(blob_left, collideParticleWithWall(Ref(*left),
                                                   Direction::east()))
        .Times(1)
        .WillRepeatedly(KillPressureInDirection(left, Direction::east()));
    EXPECT_CALL(blob_left, handleParticle(_, _)).Times(0);
    state.advance(time_delta);
}

TEST_F(StateTest, doesntMoveHostileParticlesIntoEachOther) {
    td.makeParticles({ td.lineA }, {});
    TestData<P> td2;
    td2.makeParticles({ td2.lineB }, {});
    state.emplaceBlob(0, IntVector(0, 0), 2);
    state.emplaceBlob(7, IntVector(5, 8), 4);
    B& blob_right = const_cast<B&>(state.getBlobs().at(0));
    B& blob_left = const_cast<B&>(state.getBlobs().at(7));
    P* right = td2.particle_map[IntVector(4, 10)];
    P* left = td.particle_map[IntVector(3, 10)];
    ON_CALL(blob_right, getHighestMobilityParticle())
        .WillByDefault(Return(right));
    ON_CALL(blob_left, getHighestMobilityParticle())
        .WillByDefault(Return(left));
    ON_CALL(blob_right, getParticleAt(IntVector(4, 10)))
        .WillByDefault(Return(right));
    ON_CALL(blob_left, getParticleAt(IntVector(3, 10)))
        .WillByDefault(Return(left));
    // canMove() and getPressure() will be called an undetermined amount of
    // times as the particles are sorted in the priority queue. Instead of
    // mocking the invokations, set a target and advance in order to use the
    // real particles in the mock particles.
    right->setTarget(IntVector(0, 10), 1.0f);
    left->setTarget(IntVector(5, 10), 1.0f);
    right->advance({}, std::chrono::milliseconds(1000));
    left->advance({}, std::chrono::milliseconds(1000));
    EXPECT_CALL(blob_right, collideParticleWithWall(Ref(*right),
                                                    Direction::west()))
        .Times(1)
        .WillRepeatedly(KillPressureInDirection(right, Direction::west()));
    EXPECT_CALL(blob_right, handleParticle(_, _)).Times(0);
    EXPECT_CALL(blob_left, collideParticleWithWall(Ref(*left),
                                                   Direction::east()))
        .Times(1)
        .WillRepeatedly(KillPressureInDirection(left, Direction::east()));
    EXPECT_CALL(blob_left, handleParticle(_, _)).Times(0);
    state.advance(time_delta);
}

TEST_F(StateTest, handlesHostileCollisions) {
    td.makeParticles({ td.lineA }, {});
    TestData<P> td2;
    td2.makeParticles({ td2.lineB }, {});
    state.emplaceBlob(0, IntVector(0, 0), 2);
    state.emplaceBlob(7, IntVector(5, 8), 4);
    B& blob_right = const_cast<B&>(state.getBlobs().at(0));
    B& blob_left = const_cast<B&>(state.getBlobs().at(7));
    P* right = td2.particle_map[IntVector(4, 10)];
    P* left = td.particle_map[IntVector(3, 10)];
    ON_CALL(blob_right, getHighestMobilityParticle())
        .WillByDefault(Return(right));
    ON_CALL(blob_left, getHighestMobilityParticle())
        .WillByDefault(Return(left));
    ON_CALL(blob_right, getParticleAt(IntVector(4, 10)))
        .WillByDefault(Return(right));
    ON_CALL(blob_left, getParticleAt(IntVector(3, 10)))
        .WillByDefault(Return(left));
    left->setTarget(IntVector(5, 10), 1.0f);
    left->advance({}, std::chrono::milliseconds(1000));
    EXPECT_CALL(blob_left, collideParticleWithWall(Ref(*left),
                                                   Direction::east()))
        .Times(AnyNumber())
        .WillRepeatedly(KillPressureInDirection(left, Direction::east()));
    EXPECT_CALL(blob_left, removeParticle(Ref(*left))).Times(1);
    EXPECT_CALL(blob_right, removeParticle(Ref(*right))).Times(1);
    state.advance(time_delta);
}

TEST_F(StateTest, handlesParticlesInHostileCollisionsOnlyOnce) {
    td.makeParticles({ td.lineA }, {});
    TestData<P> td2;
    td2.makeParticles({ td2.lineB }, {});
    state.emplaceBlob(0, IntVector(0, 0), 2);
    state.emplaceBlob(7, IntVector(5, 8), 4);
    B& blob_right = const_cast<B&>(state.getBlobs().at(0));
    B& blob_left = const_cast<B&>(state.getBlobs().at(7));
    P* right = td2.particle_map[IntVector(4, 10)];
    P* left = td.particle_map[IntVector(3, 10)];
    ON_CALL(blob_right, getHighestMobilityParticle())
        .WillByDefault(Return(right));
    ON_CALL(blob_left, getHighestMobilityParticle())
        .WillByDefault(Return(left));
    ON_CALL(blob_right, getParticleAt(IntVector(4, 10)))
        .WillByDefault(Return(right));
    ON_CALL(blob_left, getParticleAt(IntVector(3, 10)))
        .WillByDefault(Return(left));
    right->setTarget(IntVector(0, 10), 1.0f);
    left->setTarget(IntVector(5, 10), 1.0f);
    right->advance({}, std::chrono::milliseconds(1000));
    left->advance({}, std::chrono::milliseconds(1000));
    EXPECT_CALL(blob_right, collideParticleWithWall(Ref(*right),
                                                    Direction::west()))
        .Times(AnyNumber())
        .WillRepeatedly(KillPressureInDirection(right, Direction::west()));
    EXPECT_CALL(blob_right, handleParticle(_, _)).Times(0);
    EXPECT_CALL(blob_left, collideParticleWithWall(Ref(*left),
                                                   Direction::east()))
        .Times(AnyNumber())
        .WillRepeatedly(KillPressureInDirection(left, Direction::east()));
    EXPECT_CALL(blob_left, removeParticle(Ref(*left))).Times(1);
    EXPECT_CALL(blob_right, removeParticle(Ref(*right))).Times(1);
    state.advance(time_delta);
}

TEST_F(StateTest, doesntHandleHostileCollisionsWithinTheSameBlob) {
    td.makeParticles({ td.lineA, td.lineB }, {});
    state.emplaceBlob(0, IntVector(0, 0), 2);
    B& blob= const_cast<B&>(state.getBlobs().at(0));
    P* right = td.particle_map[IntVector(4, 10)];
    P* left = td.particle_map[IntVector(3, 10)];
    EXPECT_CALL(blob, getHighestMobilityParticle())
        .WillOnce(Return(right))
        .WillRepeatedly(Return(left));
    ON_CALL(blob, getParticleAt(IntVector(4, 10)))
        .WillByDefault(Return(right));
    ON_CALL(blob, getParticleAt(IntVector(3, 10)))
        .WillByDefault(Return(left));
    right->setTarget(IntVector(0, 10), 1.0f);
    left->setTarget(IntVector(5, 10), 1.0f);
    right->advance({}, std::chrono::milliseconds(1000));
    left->advance({}, std::chrono::milliseconds(1000));
    EXPECT_CALL(blob, handleParticle(Ref(*right), Direction::west()))
        .Times(AnyNumber())
        .WillRepeatedly(KillPressureInDirection(right, Direction::west()));
    EXPECT_CALL(blob, handleParticle(Ref(*left), Direction::east()))
        .Times(AnyNumber())
        .WillRepeatedly(KillPressureInDirection(left, Direction::east()));
    EXPECT_CALL(blob, removeParticle(Ref(*left))).Times(0);
    EXPECT_CALL(blob, removeParticle(Ref(*right))).Times(0);
    state.advance(time_delta);
}

}
}
