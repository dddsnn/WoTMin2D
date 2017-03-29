#ifndef MOCKBLOBSTATE_HPP
#define MOCKBLOBSTATE_HPP

#include "../../game/Direction.hpp"
#include "MockParticle.hpp"

#include <gmock/gmock.h>
#include <vector>
#include <unordered_map>
#include <memory>

namespace wotmin2d {
namespace mock {

using ::testing::ReturnRefOfCopy;

class MockBlobState {
    private:
    using ParticlePtr = std::shared_ptr<NiceMockParticle>;
    public:
    MockBlobState() {
        ON_CALL(*this, getHighestPressureParticle())
            .WillByDefault(ReturnRefOfCopy(ParticlePtr()));
    }
    MOCK_CONST_METHOD0(getParticles, const std::vector<ParticlePtr>&());
    MOCK_METHOD1(addParticle, void(const IntVector& position));
    MOCK_METHOD2(moveParticle, void(const ParticlePtr& particle,
                                    Direction movement_direction));
    MOCK_METHOD2(collideParticles, void(const ParticlePtr& first,
                                        const ParticlePtr& second));
    MOCK_METHOD0(advanceParticles, void());
    MOCK_CONST_METHOD0(getHighestPressureParticle, const ParticlePtr&());
};

using NiceMockBlobState = ::testing::NiceMock<MockBlobState>;

}
}

#endif
