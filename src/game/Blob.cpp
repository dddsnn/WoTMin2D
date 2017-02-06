#include "Blob.hpp"

namespace wotmin2d {

Blob::Blob() :
    particles() {
}

// bottom_right is the right- and bottom-most valid coordinate.
Blob::Blob(const Vector& center, float radius, const Vector& bottom_right) :
    particles() {
    if (radius <= 0.0) {
        return;
    }
    // Add particles by checking every Vector within the bounding box of the
    // circle whether it's less than the radius from the center.
    int rounded_radius = static_cast<int>(radius + 0.5);
    int left = std::max(0, center.getX() - rounded_radius);
    int top = std::max(0, center.getY() - rounded_radius);
    int right = std::min(center.getX() + rounded_radius, bottom_right.getX());
    int bottom = std::min(center.getY() + rounded_radius, bottom_right.getY());
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
