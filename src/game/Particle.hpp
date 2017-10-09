#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "../Config.hpp"
#include "Vector.hpp"
#include "Direction.hpp"

#include <vector>
#include <array>
#include <utility>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <unordered_set>
#include <cassert>
#include <chrono>
#include <ratio>

namespace wotmin2d {

// Forward declaration so we can make them our friends.
namespace mock {
class MockParticle;
}
namespace test {
template<class P>
class TestData;
class ParticleTest;
class BlobStateTest;
}

class Particle {
    private:
    class BlobStateKey {
        template<class P> friend class BlobState;
        friend class wotmin2d::mock::MockParticle;
        friend class wotmin2d::test::TestData<Particle>;
        friend class wotmin2d::test::ParticleTest;
        friend class wotmin2d::test::BlobStateTest;
        private:
        BlobStateKey() {}
        BlobStateKey(const BlobStateKey&) = delete;
        BlobStateKey& operator=(const BlobStateKey&) = delete;
    };
    public:
    Particle(IntVector position);
    const IntVector& getPosition() const;
    Particle* getNeighbor(Direction direction);
    const Particle* getConstNeighbor(Direction direction) const;
    void setNeighbor(BlobStateKey, Direction direction, Particle* neighbor);
    bool hasNeighbor() const;
    void move(BlobStateKey, Direction direction);
    bool hasPath(std::initializer_list<Direction> directions) const;
    const FloatVector& getPressure() const;
    Direction getPressureDirection() const;
    void advance(BlobStateKey, std::chrono::milliseconds time_delta);
    void setTarget(const IntVector& target, float target_pressure_per_second);
    void collideWith(BlobStateKey, Particle& forward_neighbor,
                     Direction collision_direction);
    void killPressureInDirection(BlobStateKey, Direction direction);
    bool canMove() const;
    void addFollowers(BlobStateKey,
                      const std::vector<Particle*>& new_followers);
    void removeFollower(BlobStateKey, Particle& follower);
    void removeLeader(BlobStateKey, Particle& leader);
    std::unordered_set<Particle*>& getFollowers(BlobStateKey);
    std::unordered_set<Particle*>& getLeaders(BlobStateKey);
    unsigned int getHealth() const;
    void damage(BlobStateKey, unsigned int amount);
    private:
    template<class C>
    void addPressureToFollowers(const C&, float magnitude);
    void addLeader(Particle& leader);
    void addFollower(Particle& follower);
    void removeLeader(Particle& leader);
    void removeFollower(Particle& follower);
    void reevaluateFollowership();
    IntVector position;
    std::array<Particle*, 4> neighbors;
    IntVector target;
    float target_pressure_per_second;
    FloatVector pressure;
    std::unordered_set<Particle*> followers;
    std::unordered_set<Particle*> leaders;
    unsigned int health;
};

template<class C>
void Particle::addPressureToFollowers(const C& followers,
                                      float magnitude) {
    for (Particle* follower: followers) {
        assert(follower != nullptr);
        FloatVector to_this = static_cast<FloatVector>(position
                                                       - follower->position);
        FloatVector to_this_pressure = to_this * (magnitude / to_this.norm());
        follower->pressure += to_this_pressure;
    }
}

}

#endif
