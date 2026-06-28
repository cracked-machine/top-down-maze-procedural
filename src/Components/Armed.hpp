#ifndef SRC_COMPONENTS_ARMED_HPP__
#define SRC_COMPONENTS_ARMED_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

namespace Game::Cmp
{

class Armed
{
public:
  enum class EpiCenter { YES, NO };

  Armed( sf::Time fuse_delay, sf::Time warning_delay, bool display_bomb_sprite, sf::Color armed_color_border, sf::Color armed_color_fill, int index,
         EpiCenter epicenter = EpiCenter::NO )
      : m_fuse_delay( fuse_delay ),
        m_warning_delay( warning_delay ),
        m_display_bomb_sprite( display_bomb_sprite ),
        m_armed_color_border( armed_color_border ),
        m_armed_color_fill( armed_color_fill ),
        m_index( index ),
        m_epicenter( epicenter )
  {
    m_fuse_delay_clock.restart();
    m_warning_delay_clock.restart();
  }

  sf::Time m_fuse_delay;
  sf::Time m_warning_delay;
  bool m_display_bomb_sprite;
  sf::Color m_armed_color_border;
  sf::Color m_armed_color_fill;
  int m_index;

  sf::Time getElapsedFuseTime() const { return m_fuse_delay_clock.getElapsedTime(); }
  sf::Clock m_fuse_delay_clock;
  sf::Time getElapsedWarningTime() const { return m_warning_delay_clock.getElapsedTime(); }
  sf::Clock m_warning_delay_clock;

  EpiCenter m_epicenter = EpiCenter::NO;

private:
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ARMED_HPP__
