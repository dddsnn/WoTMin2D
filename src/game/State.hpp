#ifndef STATE_HPP
#define STATE_HPP

#include "Blob.hpp"
#include "Vector.hpp"

#include <vector>

namespace wotmin2d {

class State {
    public:
    State(unsigned int width, unsigned int height);
    void update();
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    const std::vector<Blob>& getBlobs() const;
    private:
    const unsigned int width;
    const unsigned int height;
    std::vector<Blob> blobs;
};

}

#endif
