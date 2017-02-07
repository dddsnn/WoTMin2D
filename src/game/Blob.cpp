#include "Blob.hpp"

namespace wotmin2d {

Blob::Blob(const unsigned int& arena_width, const unsigned int& arena_height) :
    particles(),
    particle_map(),
    arena_width(arena_width),
    arena_height(arena_height) {
}

Blob::Blob(const Vector& center, float radius, const unsigned int& arena_width,
           const unsigned int& arena_height) :
    particles(),
    particle_map(),
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
                addParticle(c);
            }
        }
    }
}

const std::vector<std::shared_ptr<Particle>>& Blob::getParticles() const {
    return particles;
}

void Blob::addParticle(Vector position) {
    std::shared_ptr<Particle> particle = std::make_shared<Particle>(position);
    particles.push_back(particle);
    particle_map.insert(ParticleMap::value_type(position, particle));
}

}
