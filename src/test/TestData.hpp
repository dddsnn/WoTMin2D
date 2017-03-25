#ifndef TESTDATA_HPP
#define TESTDATA_HPP

#include "mock/MockBlobState.hpp"
#include "mock/MockParticle.hpp"
#include "../game/Vector.hpp"
#include "../game/Direction.hpp"

#include <memory>
#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <deque>

namespace wotmin2d {
namespace test {

using mock::NiceMockBlobState;
using mock::NiceMockParticle;

using ParticlePtr = std::shared_ptr<NiceMockParticle>;

/**
 * The test particle positions look as follows:
 *     012345678901234567890
 *    -----------------------
 * 20 |                     |
 * 19 |                     |
 * 18 |                     |
 * 17 |                     |
 * 16 |                     |
 * 15 |                     |
 * 14 |                     |
 * 13 |          lll        |
 * 12 |          l l        |
 * 11 |          lll        |
 * 10 |   abbbbbbb          |
 *  9 |   a                 |
 *  8 |   a                 |
 *  7 |   a                 |
 *  6 |   a                 |
 *  5 |   a           z     |
 *  4 |   akkkk             |
 *  3 |    kkkk             |
 *  2 |    kkkk             |
 *  1 |    kkkk             |
 *  0 |x         y          |
 *    -----------------------
 * x: inSouthWestCorner
 * y: onSouthBorder
 * z: inside
 * a: lineA
 * b: lineB
 * k: block
 * l: loop
 *
 * Particles can be made available in the particles and particle_map members by
 * calling makeParticles() with positions.
 */
class TestData {
    public:
    TestData() :
        particles(),
        particle_map(),
        inSouthWestCorner(0, 0),
        onSouthBorder(10, 0),
        inside(15, 5),
        lineA(),
        lineB(),
        block(),
        loop() {
        // lineA: from (3, 4) to (3, 10)
        for (int i = 4; i <= 10; i++) {
            lineA.emplace_back(IntVector(3, i));
        }
        // lineB: from (4, 10) to (10, 10)
        for (int i = 4; i <= 10; i++) {
            lineB.emplace_back(IntVector(i, 10));
        }
        // block: square from (4, 1) to (7, 4)
        for (int i = 4; i <= 7; i++) {
            for (int j = 1; j <= 4; j++) {
                block.emplace_back(IntVector(i, j));
            }
        }
        // loop: square from (10, 11) to (12, 13) with middle particle missing
        for (int i = 10; i <= 12; i++) {
            for (int j = 11; j <= 13; j++) {
                if (i == 11 && j == 12) {
                    continue;
                }
                loop.emplace_back(IntVector(i, j));
            }
        }
    }
    void makeParticles(std::initializer_list<std::vector<IntVector>> vectors,
                       std::initializer_list<IntVector> singles)
    {
        particles.clear();
        particle_map.clear();
        for (const auto& v: vectors) {
            for (const auto& p: v) {
                particle_map.emplace(p, std::make_shared<NiceMockParticle>(p));
            }
        }
        for (const auto& p: singles) {
            particle_map.emplace(p, std::make_shared<NiceMockParticle>(p));
        }
        auto getSecond = [] (std::pair<const IntVector, ParticlePtr>& v) {
                           return v.second;
                         };
        std::transform(particle_map.begin(), particle_map.end(),
                       std::back_inserter(particles), getSecond);
        // Set neighbors.
        for (const auto& p: particles) {
            for (auto d: Direction::all()) {
                auto i = particle_map.find(p->getPosition() + d.vector());
                if (i != particle_map.end()) {
                    p->setNeighbor(NiceMockParticle::MoveKey(), d, i->second);
                }
            }
        }
    }
    std::vector<ParticlePtr> particles;
    std::unordered_map<IntVector, ParticlePtr, IntVector::Hash> particle_map;
    IntVector inSouthWestCorner;
    IntVector onSouthBorder;
    IntVector inside;
    std::vector<IntVector> lineA;
    std::vector<IntVector> lineB;
    std::vector<IntVector> block;
    std::vector<IntVector> loop;
    static const unsigned int width = 21;
    static const unsigned int height = 21;
};

}
}

#endif
