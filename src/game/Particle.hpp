#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "Vector.hpp"
#include "Direction.hpp"

#include <array>
#include <utility>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <unordered_set>
#include <cassert>

namespace wotmin2d {

class Particle {
    private:
    class BlobStateKey {
        friend class BlobState;
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
    void setNeighbor(BlobStateKey, Direction direction,
                     Particle* neighbor);
    bool hasNeighbor() const;
    void move(BlobStateKey, Direction direction);
    bool hasPath(std::initializer_list<Direction> directions) const;
    const FloatVector& getPressure() const;
    Direction getPressureDirection() const;
    void advance();
    void setTarget(const IntVector& target, float target_pressure);
    void collideWith(Particle& forward_neighbor, Direction collision_direction);
    void addFollowers(BlobStateKey, const std::vector<Particle*> followers);
    bool canMove() const;
    private:
    void dividePressure(float pressure_part);
    void addLeader(Particle& leader);
    void addFollower(Particle& follower);
    void removeLeader(Particle& leader);
    void removeFollower(Particle& follower);
    void reevaluateFollowership();
    IntVector position;
    std::array<Particle*, 4> neighbors;
    IntVector target;
    float target_pressure;
    FloatVector pressure;
    std::unordered_set<Particle*> followers;
    std::unordered_set<Particle*> leaders;
};

}

#endif
