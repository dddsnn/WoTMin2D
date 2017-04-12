#ifndef MOCKPARTICLE_HPP
#define MOCKPARTICLE_HPP

#include "../../game/Direction.hpp"
#include "../../game/Vector.hpp"
#include "../../game/Particle.hpp"

#include <gmock/gmock.h>
#include <array>
#include <initializer_list>
#include <utility>
#include <chrono>

namespace wotmin2d {
namespace mock {

class MockParticle;
using NiceMockParticle = ::testing::NiceMock<MockParticle>;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRefOfCopy;

class MockParticle {
    public:
    class BlobStateKey {};
    MockParticle(IntVector position) :
        real_particle(position),
        mock_neighbors()
    {
        ON_CALL(*this, getPosition())
            .WillByDefault(Invoke(&real_particle, &Particle::getPosition));
        ON_CALL(*this, getNeighbor(_))
            .WillByDefault(Invoke(this, &MockParticle::callGetNeighbor));
        ON_CALL(*this, getConstNeighbor(_))
            .WillByDefault(Invoke(this, &MockParticle::callGetNeighbor));
        ON_CALL(*this, setNeighbor(_, _, _))
            .WillByDefault(Invoke(this, &MockParticle::callSetNeighbor));
        ON_CALL(*this, hasNeighbor())
            .WillByDefault(Invoke(&real_particle, &Particle::hasNeighbor));
        ON_CALL(*this, move(_, _))
            .WillByDefault(Invoke(this, &MockParticle::callMove));
        ON_CALL(*this, hasPath(_))
            .WillByDefault(Invoke(&real_particle, &Particle::hasPath));
        ON_CALL(*this, getPressure())
            .WillByDefault(ReturnRefOfCopy(FloatVector(0.0f, 0.0f)));
        ON_CALL(*this, getPressureDirection())
            .WillByDefault(Return(Direction::north()));
        ON_CALL(*this, advance(_))
            .WillByDefault(Invoke(&real_particle, &Particle::advance));
        ON_CALL(*this, setTarget(_, _))
            .WillByDefault(Invoke(&real_particle, &Particle::setTarget));
    }
    MOCK_CONST_METHOD0(getPosition, const IntVector&());
    MOCK_METHOD1(getNeighbor, NiceMockParticle*(Direction direction));
    MOCK_CONST_METHOD1(getConstNeighbor,
                       const NiceMockParticle*(Direction direction));
    MOCK_METHOD3(setNeighbor, void(BlobStateKey, Direction direction,
                                   NiceMockParticle* neighbor));
    MOCK_CONST_METHOD0(hasNeighbor, bool());
    MOCK_METHOD2(move, void(BlobStateKey, Direction direction));
    MOCK_CONST_METHOD1(hasPath,
                       bool(std::initializer_list<Direction> directions));
    MOCK_CONST_METHOD0(getPressure, const FloatVector&());
    MOCK_CONST_METHOD0(getPressureDirection, Direction());
    MOCK_METHOD1(advance, void(std::chrono::milliseconds time_delta));
    MOCK_METHOD2(setTarget, void(const IntVector& target,
                                 float target_pressure_per_second));
    MOCK_METHOD2(collideWith, void(NiceMockParticle& forward_neighbor,
                                   Direction collision_direction));
    MOCK_METHOD2(addFollowers, void(BlobStateKey,
                                    const std::vector<NiceMockParticle*>
                                        new_followers));
    MOCK_CONST_METHOD0(canMove, bool());
    private:
    Particle real_particle;
    std::array<NiceMockParticle*, 4> mock_neighbors;
    NiceMockParticle* callGetNeighbor(Direction direction) {
        return mock_neighbors[static_cast<Direction::val_t>(direction)];
    }
    void callSetNeighbor(BlobStateKey, Direction direction,
                         NiceMockParticle* neighbor) {
        mock_neighbors[static_cast<Direction::val_t>(direction)] = neighbor;
        if (neighbor == nullptr) {
            real_particle.setNeighbor({}, direction, nullptr);
        }
        else {
            real_particle.setNeighbor({}, direction,
                                      &(neighbor->real_particle));
        }
    }
    void callMove(BlobStateKey, Direction direction) {
        real_particle.move({}, direction);
    }
};

}
}

#endif
