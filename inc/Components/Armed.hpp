#ifndef __COMPONENTS_ARMED_HPP__
#define __COMPONENTS_ARMED_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Cmp {

class Armed
{
public:
  Armed( sf::Time fuse_delay, sf::Time warning_delay, bool display_bomb_sprite, sf::Color armed_color, int index )
      : m_fuse_delay( fuse_delay ),
        m_warning_delay( warning_delay ),
        m_display_bomb_sprite( display_bomb_sprite ),
        m_armed_color( armed_color ),
        m_index( index )
  {
    m_fuse_delay_clock.restart();
    m_warning_delay_clock.restart();
  }

  sf::Time m_fuse_delay;
  sf::Time m_warning_delay;
  bool m_display_bomb_sprite;
  sf::Color m_armed_color;
  int m_index;

  sf::Time getElapsedFuseTime() const { return m_fuse_delay_clock.getElapsedTime(); }
  sf::Clock m_fuse_delay_clock;
  sf::Time getElapsedWarningTime() const { return m_warning_delay_clock.getElapsedTime(); }
  sf::Clock m_warning_delay_clock;

private:
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_ARMED_HPP__