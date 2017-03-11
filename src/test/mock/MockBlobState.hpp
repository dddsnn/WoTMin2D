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
class MockBlobState {
    private:
    using ParticlePtr = std::shared_ptr<MockParticle>;
    public:
    MOCK_CONST_METHOD0(getParticles, const std::vector<ParticlePtr>&());
    MOCK_METHOD1(addParticle, void(const IntVector& position));
    MOCK_METHOD2(moveParticle, void(const ParticlePtr& particle,
                                    Direction movement_direction));
};

}
}

#endif
