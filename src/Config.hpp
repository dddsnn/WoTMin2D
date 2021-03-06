#ifndef CONFIG_HPP
#define CONFIG_HPP

namespace wotmin2d {

class Config {
    public:
    /**
     * The fraction of its share of the target pressure a leading particle
     * keeps for itself. Should be in [0, 1]. If 1, leader gets the same amount
     * of pressure as its followers, if 0, followers split all the pressure
     * among them.
     */
    constexpr static float target_pressure_share = 1.0f;
    /**
     * The fraction of a particles pressure in movement direction it passes on
     * to another particle it collides with. Should be in [0, 1].
     */
    constexpr static float collision_pass_on = 0.5f;
    /**
     * The fraction of a lead particles pressure it gives to new followers when
     * they are added. Should be in [0, 1].
     */
    constexpr static float boost_fraction = 0.5f;
    /**
     * The minimum pressure along an axis (i.e. the smallest maximum norm) a
     * particle can have and be considered able to move. Should be at least 1,
     * otherwise the particle would have negative pressure in the opposite
     * direction in the next time step.
     */
    constexpr static float min_directed_movement_pressure = 1.0f;
    constexpr static float min_selection_radius = 0.0f;
    constexpr static float selection_change_multiplier = 1.0f;
    constexpr static unsigned int particle_health = 100;
    static_assert(particle_health > 0, "Particles should be healty.");
    constexpr static int particle_damage = 10;
    static_assert(particle_damage > 0, "Non-positive damage ticks don't make "
                  "sense.");
    /**
     * The offset from a particle's coordinate, both in x and y direction,
     * defining the square in which other particles contribute to that
     * particle's strength. Two times the offset plus on is the length of the
     * side of the square.
     */
    constexpr static int particle_strength_offset = 4;
    static_assert(particle_strength_offset > 1, "Particle strength offsets "
                  "less than 2 mean there is no strength in numbers");
};

}

#endif
