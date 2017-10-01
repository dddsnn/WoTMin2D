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
    EXPECT_CALL(*p1, canMove())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*p2, canMove())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false));
    ON_CALL(*p1, getPressureDirection())
        .WillByDefault(Return(Direction::east()));
    EXPECT_CALL(*p1, getPressure())
        .Times(AnyNumber())
        .WillOnce(ReturnRefOfCopy(FloatVector(5.0f, 0.0f)))
        .WillOnce(ReturnRefOfCopy(FloatVector(5.0f, 0.0f)))
        .WillRepeatedly(ReturnRefOfCopy(FloatVector(0.0f, 0.0f)));
    ON_CALL(*p2, getPressureDirection())
        .WillByDefault(Return(Direction::north()));
    EXPECT_CALL(*p2, getPressure())
        .WillOnce(ReturnRefOfCopy(FloatVector(0.0f, 10.0f)))
        .WillOnce(ReturnRefOfCopy(FloatVector(0.0f, 4.0f)))
        .WillOnce(ReturnRefOfCopy(FloatVector(0.0f, 4.0f)))
        .WillRepeatedly(ReturnRefOfCopy(FloatVector(0.0f, 0.0f)));
    {
        InSequence dummy;
        EXPECT_CALL(blob2, handleParticle(Ref(*p2), Direction::north()));
        EXPECT_CALL(blob1, handleParticle(Ref(*p1), Direction::east()));
        EXPECT_CALL(blob2, handleParticle(Ref(*p2), Direction::north()));
    }
    state.advance(time_delta);
}

TEST_F(StateTest, doesntMoveParticleIntoHostileParticle) {
    td.makeParticles({ td.lineA }, {});
    state.emplaceBlob(0, IntVector(0, 0), 2);
    state.emplaceBlob(7, IntVector(5, 8), 4);
    B& blob_upper = const_cast<B&>(state.getBlobs().at(0));
    B& blob_lower = const_cast<B&>(state.getBlobs().at(7));
    P* upper = td.particle_map[IntVector(3, 10)];
    P* lower = td.particle_map[IntVector(3, 9)];
    ON_CALL(blob_upper, getHighestMobilityParticle())
        .WillByDefault(Return(upper));
    ON_CALL(blob_lower, getHighestMobilityParticle())
        .WillByDefault(Return(lower));
    ON_CALL(blob_upper, getParticleAt(IntVector(3, 10)))
        .WillByDefault(Return(upper));
    ON_CALL(blob_lower, getParticleAt(IntVector(3, 9)))
        .WillByDefault(Return(lower));
    lower->setTarget(IntVector(3, 15), 1.0f);
    lower->advance({}, std::chrono::milliseconds(1000));
    EXPECT_CALL(blob_lower, collideParticleWithWall(Ref(*lower),
                                                    Direction::north()))
        .Times(1)
        .WillRepeatedly(KillPressureInDirection(lower, Direction::north()));
    EXPECT_CALL(blob_lower, handleParticle(_, _)).Times(0);
    state.advance(time_delta);
}

TEST_F(StateTest, doesntMoveHostileParticlesIntoEachOther) {
    td.makeParticles({ td.lineA }, {});
    state.emplaceBlob(0, IntVector(0, 0), 2);
    state.emplaceBlob(7, IntVector(5, 8), 4);
    B& blob_upper = const_cast<B&>(state.getBlobs().at(0));
    B& blob_lower = const_cast<B&>(state.getBlobs().at(7));
    P* upper = td.particle_map[IntVector(3, 10)];
    P* lower = td.particle_map[IntVector(3, 9)];
    ON_CALL(blob_upper, getHighestMobilityParticle())
        .WillByDefault(Return(upper));
    ON_CALL(blob_lower, getHighestMobilityParticle())
        .WillByDefault(Return(lower));
    ON_CALL(blob_upper, getParticleAt(IntVector(3, 10)))
        .WillByDefault(Return(upper));
    ON_CALL(blob_lower, getParticleAt(IntVector(3, 9)))
        .WillByDefault(Return(lower));
    // canMove() and getPressure() will be called an undetermined amount of
    // times as the particles are sorted in the priority queue. Instead of
    // mocking the invokations, set a target and advance in order to use the
    // real particles in the mock particles.
    upper->setTarget(IntVector(3, 0), 1.0f);
    lower->setTarget(IntVector(3, 15), 1.0f);
    upper->advance({}, std::chrono::milliseconds(1000));
    lower->advance({}, std::chrono::milliseconds(1000));
    EXPECT_CALL(blob_upper, collideParticleWithWall(Ref(*upper),
                                                    Direction::south()))
        .Times(1)
        .WillRepeatedly(KillPressureInDirection(upper, Direction::south()));
    EXPECT_CALL(blob_upper, handleParticle(_, _)).Times(0);
    EXPECT_CALL(blob_lower, collideParticleWithWall(Ref(*lower),
                                                    Direction::north()))
        .Times(1)
        .WillRepeatedly(KillPressureInDirection(lower, Direction::north()));
    EXPECT_CALL(blob_lower, handleParticle(_, _)).Times(0);
    state.advance(time_delta);
}

}
}
