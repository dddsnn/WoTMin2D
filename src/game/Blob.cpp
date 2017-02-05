#include "Blob.hpp"

namespace wotmin2d {

Blob::Blob() :
    particles() {
}

Blob::Blob(unsigned int center_x, unsigned int center_y, float radius,
           unsigned int max_x, unsigned int max_y) :
    particles() {
    // Add particles by checking every coordinate within the bounding box of the
    // circle whether it's less than the radius from the center.
    unsigned int rounded_radius = static_cast<unsigned int>(radius + 0.5);
    unsigned int left = rounded_radius > center_x ? 0 : center_x
                                                        - rounded_radius;
    unsigned int top = rounded_radius > center_y ? 0 : center_y
                                                       - rounded_radius;
    unsigned int right = std::min(center_x + rounded_radius, max_x);
    unsigned int bottom = std::min(center_y + rounded_radius, max_y);
    float squared_radius = radius * radius;
    for (unsigned int i = left; i <= right; i++) {
        for (unsigned int j = top ; j <= bottom; j++) {
            int diff_x = i - center_x;
            int diff_y = j - center_y;
            if (diff_x * diff_x + diff_y * diff_y < squared_radius) {
                // Add particle.
                particles.push_back(std::make_shared<Particle>(i, j));
            }
        }
    }
}

const std::vector<std::shared_ptr<Particle>>& Blob::getParticles() const {
    return particles;
}

}
