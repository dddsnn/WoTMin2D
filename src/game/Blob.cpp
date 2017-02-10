#include "Blob.hpp"

namespace wotmin2d {

Blob::Blob(const unsigned int& arena_width, const unsigned int& arena_height) :
    particles(),
    particle_map(),
    arena_width(arena_width),
    arena_height(arena_height) {
}

Blob::Blob(const IntVector& center, float radius,
           const unsigned int& arena_width, const unsigned int& arena_height) :
    particles(),
    particle_map(),
    arena_width(arena_width),
    arena_height(arena_height) {
    if (radius <= 0.0) {
        return;
    }
    // Add particles by checking every vector within the bounding box of the
    // circle whether it's less than the radius from the center.
    int rounded_radius = static_cast<int>(radius + 0.5);
    int left = std::max(0, center.getX() - rounded_radius);
    int top = std::max(0, center.getY() - rounded_radius);
    int right = std::min(center.getX() + rounded_radius,
                         static_cast<int>(arena_width - 1));
    int bottom = std::min(center.getY() + rounded_radius,
                          static_cast<int>(arena_height - 1));
    float squared_radius = radius * radius;
    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            IntVector coordinate(x, y);
            IntVector center_to_coordinate = coordinate - center;
            int dot_product = center_to_coordinate.dot(center_to_coordinate);
            if (dot_product < squared_radius) {
                // Add particle.
                addParticle(coordinate);
            }
        }
    }
}

const std::vector<std::shared_ptr<Particle>>& Blob::getParticles() const {
    return particles;
}

void Blob::addParticle(IntVector position) {
    std::shared_ptr<Particle> particle = std::make_shared<Particle>(position);
    particles.push_back(particle);
    particle_map.insert(ParticleMap::value_type(position, particle));
    // Make potential neighbors aware of the new particle.
    for (const Direction& direction: Direction::directions()) {
        auto neighbor_iter = particle_map.find(position + direction.vector());
        if (neighbor_iter == particle_map.end()) {
            continue;
        }
        std::shared_ptr<Particle>& neighbor = (*neighbor_iter).second;
        neighbor->neighbor(direction) = particle;
    }
}

void Blob::advance() {
    for (std::shared_ptr<Particle>& particle: particles) {
        // TODO can these be null?
        advanceParticle(*particle);
    }
}

void Blob::advanceParticle(Particle& particle) {
    using Movement = Particle::Movement;
    Movement movement = particle.getMovement();
    if (!movement.second) {
        // Particle doesn't want to move.
        return;
    }
    Direction& forward_direction = movement.first;
    std::shared_ptr<Particle>& forward_neighbor
        = particle.neighbor(forward_direction);
    if (forward_neighbor != nullptr) {
        // Movement is obstructed. Only collide.
        particle.collideWith(*forward_neighbor);
        return;
    }
    particle.move(forward_direction);
}

}
