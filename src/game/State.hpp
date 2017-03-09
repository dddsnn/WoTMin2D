#ifndef STATE_HPP
#define STATE_HPP

#include "Blob.hpp"
#include "Vector.hpp"

#include <vector>

namespace wotmin2d {

class State {
    public:
    State(unsigned int arena_width, unsigned int arena_height);
    void update();
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    const std::vector<Blob<>>& getBlobs() const;
    private:
    const unsigned int arena_width;
    const unsigned int arena_height;
    std::vector<Blob<>> blobs;
};

}

#endif
