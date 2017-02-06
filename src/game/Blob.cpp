#include "Blob.hpp"

namespace wotmin2d {

Blob::Blob(const unsigned int& arena_width, const unsigned int& arena_height) :
    particles(),
    arena_width(arena_width),
    arena_height(arena_height) {
}

// bottom_right is the right- and bottom-most valid coordinate.
Blob::Blob(const Vector& center, float radius, const unsigned int& arena_width,
           const unsigned int& arena_height) :
    particles(),
    arena_width(arena_width),
    arena_height(arena_height) {
    if (radius <= 0.0) {
        return;
    }
    // Add particles by checking every Vector within the bounding box of the
    // circle whether it's less than the radius from the center.
    int rounded_radius = static_cast<int>(radius + 0.5);
    int left = std::max(0, center.getX() - rounded_radius);
    int top = std::max(0, center.getY() - rounded_radius);
    int right = std::min(center.getX() + rounded_radius,
                         static_cast<int>(arena_width - 1));
    int bottom = std::min(center.getY() + rounded_radius,
                          static_cast<int>(arena_height - 1));
    float squared_radius = radius * radius;
    for (Vector c(left, top); c.y() <= bottom; c.y()++) {
        for (c.x() = left; c.x() <= right; c.x()++) {
            Vector center_to_c = c - center;
            if (center_to_c.dot(center_to_c) < squared_radius) {
                // Add particle.
                particles.push_back(std::make_shared<Particle>(c));
            }
        }
    }
}

const std::vector<std::shared_ptr<Particle>>& Blob::getParticles() const {
    return particles;
}

}
