#ifndef MOCKPARTICLE_HPP
#define MOCKPARTICLE_HPP

#include "../../game/Direction.hpp"
#include "../../game/Vector.hpp"
#include "../../game/Particle.hpp"
#include "../../game/ParticlePositionState.hpp"

#include <gmock/gmock.h>
#include <memory>
#include <array>
#include <initializer_list>
#include <utility>

namespace wotmin2d {
namespace mock {

class MockParticle;
using NiceMockParticle = ::testing::NiceMock<MockParticle>;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class MockParticle {
    public:
    class MoveKey {};
    using Movement = std::pair<Direction, bool>;
    MockParticle(IntVector position) : position_state(position) {
        using PPS = ParticlePositionState<NiceMockParticle>;
        ON_CALL(*this, getPosition())
            .WillByDefault(Invoke(&position_state, &PPS::getPosition));
        ON_CALL(*this, move(_, _))
            .WillByDefault(Invoke(this, &MockParticle::callMove));
        ON_CALL(*this, getNeighbor(_))
            .WillByDefault(Invoke(&position_state, &PPS::getNeighbor));
        ON_CALL(*this, setNeighbor(_, _, _))
            .WillByDefault(Invoke(this, &MockParticle::callSetNeighbor));
        ON_CALL(*this, hasPath(_))
            .WillByDefault(Invoke(&position_state, &PPS::hasPath));
        ON_CALL(*this, getMovement())
            .WillByDefault(Return(std::make_pair(Direction::north(), false)));
    }
    MOCK_CONST_METHOD0(getPosition, const IntVector&());
    MOCK_CONST_METHOD1(getNeighbor, const std::shared_ptr<NiceMockParticle>&
                                        (Direction direction));
    MOCK_METHOD3(setNeighbor, void(MoveKey, Direction direction,
                                   const std::shared_ptr<NiceMockParticle>&
                                       neighbor));
    MOCK_METHOD2(move, void(MoveKey, Direction direction));
    MOCK_METHOD1(hasPath, bool(std::initializer_list<Direction> directions));
    MOCK_CONST_METHOD0(getMovement, Movement());
    MOCK_METHOD0(advance, void());
    MOCK_METHOD2(setTarget, void(const IntVector& target,
                                 float target_pressure));
    MOCK_METHOD1(collideWith, void(NiceMockParticle& forward_neighbor));
    private:
    ParticlePositionState<NiceMockParticle> position_state;
    void callSetNeighbor(MoveKey, Direction direction,
                         const std::shared_ptr<NiceMockParticle>& neighbor) {
        position_state.setNeighbor(direction, neighbor);
    }
    void callMove(MoveKey, Direction direction) {
        const IntVector& vector = direction.vector();
        position_state.move(vector);
    }
};

}
}

#endif
