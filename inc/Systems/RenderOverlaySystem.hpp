#ifndef __SYS_RENDEROVERSYSTEM_HPP__
#define __SYS_RENDEROVERSYSTEM_HPP__

#include <Systems/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

class RenderOverlaySystem : public RenderSystem
{
public:
  RenderOverlaySystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg )
      : RenderSystem( reg ) {};

  void render_entt_distance_set_overlay( sf::Vector2f pos );
  void render_bomb_radius_overlay( int radius_value, sf::Vector2f pos );
  void render_bomb_overlay( int bomb_count, sf::Vector2f pos );
  void render_health_overlay( float health_value, sf::Vector2f pos, sf::Vector2f size );
  void render_water_level_meter_overlay( float water_level, sf::Vector2f pos, sf::Vector2f size );

private:
  // overlay text
  sf::Text healthlvl_meter_text{ m_font, "Health:", 30 };
  sf::Text waterlvl_meter_text{ m_font, "Flood:", 30 };
  sf::Text bomb_inventory_text{ m_font, "Bombs:", 30 };
  sf::Text bomb_radius_text{ m_font, "Blast Radius:", 30 };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDEROVERSYSTEM_HPP__