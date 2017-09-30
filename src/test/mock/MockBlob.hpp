#ifndef MOCKBLOB_HPP
#define MOCKBLOB_HPP

#include "../../game/Direction.hpp"
#include "../../game/Blob.hpp"
#include "../../game/BlobState.hpp"
#include "MockParticle.hpp"

#include <gmock/gmock.h>
#include <chrono>

namespace wotmin2d {
namespace mock {

class MockBlob {
    private:
    using P = NiceMockParticle;
    public:
    MockBlob() {}
    MockBlob(const IntVector&, float, unsigned int, unsigned int) {}
    MOCK_CONST_METHOD0(getParticles,
                       const typename BlobState<P>::ParticleSet&());
    MOCK_METHOD1(advanceParticles, void(std::chrono::milliseconds time_delta));
    MOCK_CONST_METHOD0(getHighestMobilityParticle, P*());
    MOCK_METHOD4(setTarget, void(const IntVector& target,
                                 float pressure_per_second,
                                 const IntVector& center, float radius));
    MOCK_METHOD2(collideParticleWithWall,
                 void(P& particle, Direction collision_direction));
    MOCK_METHOD2(handleParticle,
                 void(P& particle, Direction collision_direction));
};

// Subclass NiceMock to allow copying. This is necessary to allow them to be
// added to a vector.
class NiceMockBlob : public ::testing::NiceMock<MockBlob> {
    public:
    NiceMockBlob(const NiceMockBlob&) : NiceMock<MockBlob>() {}
    NiceMockBlob(const IntVector& center, float radius,
                 unsigned int arena_width, unsigned int arena_height) :
        NiceMock<MockBlob>(center, radius, arena_width, arena_height) {}
};

}
}

#endif
