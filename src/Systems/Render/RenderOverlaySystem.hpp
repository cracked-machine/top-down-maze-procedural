#ifndef __SYS_RENDEROVERSYSTEM_HPP__
#define __SYS_RENDEROVERSYSTEM_HPP__

#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
{

class RenderOverlaySystem : public RenderSystem
{
public:
  RenderOverlaySystem( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : RenderSystem( window, sprite_factory, sound_bank )
  {
    // Pre-warm font texture atlas with all glyphs used in debug overlays
    sf::Text warmup_30( m_font, "0123456789 [](),:.-=xyzEPONCSabcdefghijklmnopqrstuvwXYZ INFINITE", 30 );
    warmup_30.setOutlineThickness( 2.f );
    std::ignore = warmup_30.getGlobalBounds(); // Force glyph generation

    sf::Text warmup_20( m_font, "0123456789 [](),:.-=xyzEPONCSabcdefghijklmnopqrstuvwXYZ", 20 );
    warmup_20.setOutlineThickness( 1.f );
    std::ignore = warmup_20.getGlobalBounds(); // Force glyph generation
    SPDLOG_DEBUG( "RenderOverlaySystem initialized" );

    sf::Text warmup_15( m_font, "0123456789 [](),:.-=xyzEPONCSabcdefghijklmnopqrstuvwXYZ", 15 );
    warmup_15.setOutlineThickness( 1.f );
    std::ignore = warmup_15.getGlobalBounds(); // Force glyph generation
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
  void render_npc_list_overlay( sf::Vector2f pos );
  void render_obstacle_markers();
  void render_player_distances();
  void render_scan_detection_bounds();
  void render_lerp_positions();

private:
  // restrict the debug data update to every 1 second (optimization)
  const sf::Time m_debug_update_interval{ sf::milliseconds( 1000 ) };
  sf::Clock m_debug_update_timer;

  // overlay text
  sf::Text m_distance_text{ m_font, "", 7 };
  sf::Text m_healthlvl_meter_text{ m_font, "Health:", 30 };
  sf::Text m_waterlvl_meter_text{ m_font, "Flood:", 30 };
  sf::Text m_bomb_inventory_text{ m_font, "Bombs:", 30 };
  sf::Text m_bomb_radius_text{ m_font, "Blast Radius:", 30 };
  sf::Text m_player_position_text{ m_font, "Player Pos:", 30 };
  sf::Text m_mouse_position_text{ m_font, "", 30 };
  sf::Text m_stats_text1{ m_font, "", 30 };
  sf::Text m_stats_text2{ m_font, "", 30 };
  sf::Text m_stats_text3{ m_font, "", 30 };
  std::map<unsigned int, sf::Text> m_npc_list_text{};
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDEROVERSYSTEM_HPP__