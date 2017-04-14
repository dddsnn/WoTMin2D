#ifndef TESTDATA_HPP
#define TESTDATA_HPP

#include "mock/MockBlobState.hpp"
#include "mock/MockParticle.hpp"
#include "../game/Vector.hpp"
#include "../game/Direction.hpp"

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

/**
 * The test particle positions look as follows:
 *     012345678901234567890
 *    -----------------------
 * 20 |                    w|
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
 * w: inNorthEastCorner
 * a: lineA
 * b: lineB
 * k: block
 * l: loop
 *
 * Particles can be made available in the particles and particle_map members by
 * calling makeParticles() with positions.
 */
template<class P>
class TestData {
    private:
    void deallocateParticles() {
        for (P* p: particles) {
            delete p;
        }
    }
    public:
    TestData() :
        particles(),
        particle_map(),
        inSouthWestCorner(0, 0),
        inNorthEastCorner(width - 1, height - 1),
        onSouthBorder(10, 0),
        inside(15, 5),
        lineA(),
        lineB(),
        block(),
        loop() {
        // lineA: from (3, 4) to (3, 10)
        for (int i = 4; i <= 10; i++) {
            lineA.emplace_back(3, i);
        }
        // lineB: from (4, 10) to (10, 10)
        for (int i = 4; i <= 10; i++) {
            lineB.emplace_back(i, 10);
        }
        // block: square from (4, 1) to (7, 4)
        for (int i = 4; i <= 7; i++) {
            for (int j = 1; j <= 4; j++) {
                block.emplace_back(i, j);
            }
        }
        // loop: square from (10, 11) to (12, 13) with middle particle missing
        for (int i = 10; i <= 12; i++) {
            for (int j = 11; j <= 13; j++) {
                if (i == 11 && j == 12) {
                    continue;
                }
                loop.emplace_back(i, j);
            }
        }
    }
    ~TestData() {
        deallocateParticles();
    }
    void makeParticles(std::initializer_list<std::vector<IntVector>> vectors,
                       std::initializer_list<IntVector> singles)
    {
        deallocateParticles();
        particles.clear();
        particle_map.clear();
        for (const auto& vector: vectors) {
            for (const auto& position: vector) {
                P* particle = new P(position);
                particles.push_back(particle);
                particle_map.emplace(position, particle);
            }
        }
        for (const auto& position: singles) {
            P* particle = new P(position);
            particles.push_back(particle);
            particle_map.emplace(position, particle);
        }
        // Set neighbors.
        for (P* particle: particles) {
            for (auto direction: Direction::all()) {
                auto i = particle_map.find(particle->getPosition()
                                           + direction.vector());
                if (i != particle_map.end()) {
                    particle->setNeighbor({}, direction, i->second);
                }
            }
        }
    }
    std::vector<P*> particles;
    std::unordered_map<IntVector, P*, IntVector::Hash> particle_map;
    IntVector inSouthWestCorner;
    IntVector inNorthEastCorner;
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
