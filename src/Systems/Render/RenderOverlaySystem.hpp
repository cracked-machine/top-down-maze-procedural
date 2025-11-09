#ifndef __SYS_RENDEROVERSYSTEM_HPP__
#define __SYS_RENDEROVERSYSTEM_HPP__

#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

class RenderOverlaySystem : public RenderSystem
{
public:
  RenderOverlaySystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                       Audio::SoundBank &sound_bank )
      : RenderSystem( reg, window, sprite_factory, sound_bank )
  {
    SPDLOG_DEBUG( "RenderOverlaySystem initialized" );
  };

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void render_entt_distance_set_overlay( sf::Vector2f pos );
  void render_ui_background_overlay( sf::Vector2f pos, sf::Vector2f size );
  void render_health_overlay( float health_value, sf::Vector2f pos, sf::Vector2f size );
  void render_weapons_meter_overlay( float weapons_level, sf::Vector2f pos, sf::Vector2f size );
  void render_bomb_overlay( int bomb_count, int radius_value, sf::Vector2f pos );
  void render_player_candles_overlay( unsigned int candle_count, sf::Vector2f pos );
  void render_key_count_overlay( unsigned int key_count, sf::Vector2f pos );
  void render_relic_count_overlay( unsigned int relic_count, sf::Vector2f pos );
  void render_water_level_meter_overlay( float water_level, sf::Vector2f pos, sf::Vector2f size );

  void render_player_position_overlay( sf::Vector2f player_position, sf::Vector2f pos );
  void render_mouse_position_overlay( sf::Vector2f mouse_position, sf::Vector2f pos );
  void render_stats_overlay( sf::Vector2f pos1, sf::Vector2f pos2, sf::Vector2f pos3 );

private:
  // overlay text
  sf::Text healthlvl_meter_text{ m_font, "Health:", 30 };
  sf::Text waterlvl_meter_text{ m_font, "Flood:", 30 };
  sf::Text bomb_inventory_text{ m_font, "Bombs:", 30 };
  sf::Text bomb_radius_text{ m_font, "Blast Radius:", 30 };
  sf::Text player_position_text{ m_font, "Player Pos:", 30 };
  sf::Text stats_text{ m_font, "", 30 };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDEROVERSYSTEM_HPP__