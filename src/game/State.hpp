#ifndef STATE_HPP
#define STATE_HPP

#include "Blob.hpp"
#include "Vector.hpp"
#include "../Config.hpp"

#include <vector>
#include <chrono>

namespace wotmin2d {

class State {
    public:
    using PlayerId = Blob<>::PlayerId;
    State(unsigned int arena_width, unsigned int arena_height);
    void advance(std::chrono::milliseconds time_delta);
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    const std::vector<Blob<>>& getBlobs() const;
    void setTarget(PlayerId player, const IntVector& target);
    private:
    const unsigned int arena_width;
    const unsigned int arena_height;
    std::vector<Blob<>> blobs;
};

}

#endif
