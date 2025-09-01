#ifndef __COMPONENTS_ARMED_HPP__
#define __COMPONENTS_ARMED_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Cmp {

class Armed {
public:
    Armed( sf::Time detonation_delay, bool display_bomb_sprite, sf::Color armed_color, int index )
    : 
        m_detonation_delay(detonation_delay), 
        m_display_bomb_sprite(display_bomb_sprite),
        m_armed_color(armed_color),
        m_index(index)
    { 
        m_clock.restart(); 
    }

    sf::Time m_detonation_delay;
    bool m_display_bomb_sprite;
    sf::Color m_armed_color;
    int m_index;

    sf::Time getElapsedTime() const { return m_clock.getElapsedTime(); }
    sf::Clock m_clock;
private:
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_ARMED_HPP__