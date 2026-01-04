#ifndef __SYS_RENDEROVERSYSTEM_HPP__
#define __SYS_RENDEROVERSYSTEM_HPP__

#include <Components/ZOrderValue.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Utils.hpp>

#include <set>

namespace ProceduralMaze::Sys
{

class RenderOverlaySystem : public RenderSystem
{
public:
  RenderOverlaySystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : RenderSystem( reg, window, sprite_factory, sound_bank )
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
  void render_cadaver_count_overlay( unsigned int cadaver_count, sf::Vector2f pos );
  void render_wealth_overlay( unsigned int wealth_value, sf::Vector2f pos );
  void render_water_level_meter_overlay( float water_level, sf::Vector2f pos, sf::Vector2f size );

  void render_player_position_overlay( sf::Vector2f player_position, sf::Vector2f pos );
  void render_mouse_position_overlay( sf::Vector2f mouse_position, sf::Vector2f pos );
  void render_stats_overlay( sf::Vector2f pos1, sf::Vector2f pos2, sf::Vector2f pos3, sf::Vector2f pos4 );
  void render_zorder_values_overlay( sf::Vector2f pos, std::vector<ZOrder> &zorder_queue, std::set<Sprites::SpriteMetaType> exclusions = {} );
  void render_npc_list_overlay( sf::Vector2f pos );
  void render_obstacle_markers();
  void render_player_distances();
  void render_scan_detection_bounds();
  void render_lerp_positions();

  void render_crypt_maze_timer( sf::Vector2f pos, unsigned int size );

  template <typename Component>
  void render_square_for_entity( entt::entity entity, sf::Color square_color = sf::Color::Red, float square_thickness = 1.f )
  {
    if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
    {
      auto pos_cmp = getReg().try_get<Cmp::Position>( entity );
      auto requested_cmp = getReg().try_get<Component>( entity );
      if ( pos_cmp && requested_cmp )
      {
        sf::RectangleShape rectangle;
        rectangle.setSize( Constants::kGridSquareSizePixelsF );
        rectangle.setPosition( pos_cmp->position );
        rectangle.setFillColor( sf::Color::Transparent );
        rectangle.setOutlineColor( square_color );
        rectangle.setOutlineThickness( square_thickness );
        m_window.draw( rectangle );
      }
    }
  }

  template <typename Component>
  void render_square_for_vector2f_cmp( sf::Color square_color = sf::Color::Red, float square_thickness = 1.f )
  {
    auto requested_view = getReg().view<Component>();
    for ( auto [entity, requested_cmp] : requested_view.each() )
    {
      if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
      {
        sf::RectangleShape rectangle;
        rectangle.setSize( Constants::kGridSquareSizePixelsF );
        rectangle.setPosition( requested_cmp );
        rectangle.setFillColor( sf::Color::Transparent );
        rectangle.setOutlineColor( square_color );
        rectangle.setOutlineThickness( square_thickness );
        m_window.draw( rectangle );
      }
    }
  }

  template <typename Component>
  void render_square_for_floatrect_cmp( sf::Color square_color = sf::Color::Red, float square_thickness = 1.f )
  {
    auto requested_view = getReg().view<Component>();
    for ( auto [entity, requested_cmp] : requested_view.each() )
    {
      if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
      {
        sf::RectangleShape rectangle;
        rectangle.setSize( requested_cmp.size );
        rectangle.setPosition( requested_cmp.position );
        rectangle.setFillColor( sf::Color::Transparent );
        rectangle.setOutlineColor( square_color );
        rectangle.setOutlineThickness( square_thickness );
        m_window.draw( rectangle );
      }
    }
  }

  template <typename Component>
  void render_zorder_value( sf::Color text_color = sf::Color::White, int precision = 1 )
  {
    // Save the current view
    sf::View previous_view = m_window.getView();
    // Set the game view for world-space rendering
    m_window.setView( RenderSystem::s_game_view );

    auto requested_view = getReg().view<Component>();
    for ( auto [entity, requested_cmp] : requested_view.each() )
    {
      if ( getReg().all_of<Cmp::Position, Cmp::ZOrderValue>( entity ) )
      {
        auto &pos_cmp = getReg().get<Cmp::Position>( entity );
        auto zorder_cmp = getReg().get<Cmp::ZOrderValue>( entity );
        std::stringstream ss;
        ss << std::fixed << std::setprecision( precision ) << zorder_cmp.getZOrder();
        sf::Text m_z_text{ m_font, ss.str(), 7 };
        m_z_text.setFillColor( text_color );
        m_z_text.setPosition( { pos_cmp.position.x, pos_cmp.position.y } );
        m_z_text.setOutlineColor( sf::Color::Black );
        m_z_text.setOutlineThickness( 0.5f );
        m_window.draw( m_z_text );
      }
    }

    // Restore the previous view
    m_window.setView( previous_view );
  }

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
  sf::Text m_stats_text4{ m_font, "", 30 };
  std::map<unsigned int, sf::Text> m_npc_list_text{};
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDEROVERSYSTEM_HPP__