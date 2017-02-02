#ifndef STATE_HPP
#define STATE_HPP

#include "Blob.hpp"

#include <vector>

namespace wotmin2d {

class State {
    public:
    void update();
    unsigned int getWidth();
    unsigned int getHeight();
    const std::vector<Blob>& getBlobs() const;
};

}

#endif
