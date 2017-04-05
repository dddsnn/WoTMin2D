#ifndef MOCKBLOBSTATE_HPP
#define MOCKBLOBSTATE_HPP

#include "../../game/Direction.hpp"
#include "MockParticle.hpp"

#include <gmock/gmock.h>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace wotmin2d {
namespace mock {

using ::testing::Return;

class MockBlobState {
    private:
    using P = NiceMockParticle;
    public:
    MockBlobState() {
        ON_CALL(*this, getHighestMobilityParticle())
            .WillByDefault(Return(nullptr));
    }
    MOCK_CONST_METHOD0(getParticles, const std::vector<P*>&());
    MOCK_METHOD1(addParticle, void(const IntVector& position));
    MOCK_METHOD2(moveParticle, void(const P& particle,
                                    Direction movement_direction));
    MOCK_METHOD3(collideParticles, void(P& first, P& second,
                                        Direction collision_direction));
    MOCK_METHOD1(advanceParticles, void(std::chrono::milliseconds time_delta));
    MOCK_METHOD0(getHighestMobilityParticle, P*());
    MOCK_METHOD2(addParticleFollowers, void(P& leader,
                                            const std::vector<P*> followers));
};

using NiceMockBlobState = ::testing::NiceMock<MockBlobState>;

}
}

#endif
