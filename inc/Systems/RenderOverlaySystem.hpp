#ifndef __SYS_RENDEROVERSYSTEM_HPP__
#define __SYS_RENDEROVERSYSTEM_HPP__

#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <Systems/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

class RenderOverlaySystem : public RenderSystem
{
public:
  RenderOverlaySystem( ProceduralMaze::SharedEnttRegistry reg )
      : RenderSystem( reg ) {};

  void render_entt_distance_set_overlay( sf::Vector2f pos );
  void render_bomb_radius_overlay( int radius_value, sf::Vector2f pos );
  void render_bomb_overlay( int bomb_count, sf::Vector2f pos );
  void render_health_overlay( float health_value, sf::Vector2f pos, sf::Vector2f size );
  void render_water_level_meter_overlay( float water_level, sf::Vector2f pos, sf::Vector2f size );
  void render_player_position_overlay( sf::Vector2f player_position, sf::Vector2f pos );
  void render_player_score_overlay( unsigned int player_score, sf::Vector2f pos );
  void render_mouse_position_overlay( sf::Vector2f mouse_position, sf::Vector2f pos );

private:
  // overlay text
  sf::Text healthlvl_meter_text{ m_font, "Health:", 30 };
  sf::Text waterlvl_meter_text{ m_font, "Flood:", 30 };
  sf::Text bomb_inventory_text{ m_font, "Bombs:", 30 };
  sf::Text bomb_radius_text{ m_font, "Blast Radius:", 30 };
  sf::Text player_position_text{ m_font, "Player Pos:", 30 };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDEROVERSYSTEM_HPP__