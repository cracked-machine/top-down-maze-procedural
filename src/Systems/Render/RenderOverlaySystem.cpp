#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>

#include <Components/CorruptionCell.hpp>
#include <Components/HazardFieldCell.hpp>
#include <Components/Position.hpp>
#include <Components/SinkholeCell.hpp>
#include <SFML/System/Vector2.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>

namespace ProceduralMaze::Sys {

void RenderOverlaySystem::render_entt_distance_set_overlay( sf::Vector2f pos )
{
  if ( !m_show_path_distances ) return;

  auto entt_distance_map_view = m_reg->view<Cmp::EnttDistanceMap>();
  int entt_distance_set = 0;
  for ( auto [e, distance_map] : entt_distance_map_view.each() )
  {
    sf::Text distance_text( m_font, "", 30 );
    distance_text.setFillColor( sf::Color::White );
    distance_text.setOutlineColor( sf::Color::Black );
    distance_text.setOutlineThickness( 2.f );

    if ( distance_map.empty() ) { continue; }
    else
    {
      std::stringstream ss;
      ss << "NPC Entity #" << entt::to_integral( e ) << " - ";
      distance_text.setPosition( pos + sf::Vector2f{ 0, entt_distance_set * 30.f } );

      for ( auto it = distance_map.begin(); it != distance_map.end(); ++it )
      {
        ss << " " << entt::to_integral( it->first ) << ":" << it->second << ",";
      }

      distance_text.setString( ss.str() );
      m_window.draw( distance_text );
    }
    entt_distance_set++;
  }
}

void RenderOverlaySystem::render_ui_background_overlay( sf::Vector2f pos, sf::Vector2f size )
{
  auto ui_background = sf::RectangleShape( size );
  ui_background.setPosition( pos );
  ui_background.setFillColor( sf::Color( 48, 48, 64, 128 ) );
  m_window.draw( ui_background );

  auto ui_edge = sf::RectangleShape( size );
  ui_edge.setPosition( pos );
  ui_edge.setFillColor( sf::Color( sf::Color::Transparent ) );
  ui_edge.setOutlineColor( sf::Color::Black );
  ui_edge.setOutlineThickness( 5.f );
  m_window.draw( ui_edge );
}

void RenderOverlaySystem::render_health_overlay( float health_value, sf::Vector2f pos, sf::Vector2f size )
{

  auto sprite_metatype = "ICONS";
  auto position = sf::FloatRect{ pos, kGridSquareSizePixelsF };
  auto sprite_index = 0; // health icon
  auto scale = sf::Vector2f( 2.f, 2.f );
  RenderSystem::safe_render_sprite( sprite_metatype, position, sprite_index, scale );

  // bar fill
  sf::Vector2f healthbar_offset{ 50.f, 8.f };
  auto healthbar = sf::RectangleShape( { ( ( size.x / 100 ) * health_value ), size.y } );
  healthbar.setPosition( pos + healthbar_offset );
  healthbar.setFillColor( sf::Color::Red );
  m_window.draw( healthbar );

  // bar outline
  auto healthbar_border = sf::RectangleShape( size );
  healthbar_border.setPosition( pos + healthbar_offset );
  healthbar_border.setFillColor( sf::Color::Transparent );
  healthbar_border.setOutlineColor( sf::Color::Black );
  healthbar_border.setOutlineThickness( 5.f );
  m_window.draw( healthbar_border );
}

void RenderOverlaySystem::render_weapons_meter_overlay( float new_weapon_level, sf::Vector2f pos, sf::Vector2f size )
{
  auto sprite_metatype = "ICONS";
  auto position = sf::FloatRect{ pos, kGridSquareSizePixelsF };
  auto sprite_index = 1; // hammer icon
  auto scale = sf::Vector2f( 2.f, 2.f );
  RenderSystem::safe_render_sprite( sprite_metatype, position, sprite_index, scale );

  // bar fill - weapons level is out of 100, but the bar is 200 pixels wide
  sf::Vector2f weapons_meter_offset{ 50.f, 8.f };
  auto weaponsbar = sf::RectangleShape( { new_weapon_level * 2, size.y } );
  weaponsbar.setPosition( pos + weapons_meter_offset );
  weaponsbar.setFillColor( sf::Color::Green );
  m_window.draw( weaponsbar );

  // bar outline
  auto weaponsbar_border = sf::RectangleShape( size );
  weaponsbar_border.setPosition( pos + weapons_meter_offset );
  weaponsbar_border.setFillColor( sf::Color::Transparent );
  weaponsbar_border.setOutlineColor( sf::Color::Black );
  weaponsbar_border.setOutlineThickness( 5.f );
  m_window.draw( weaponsbar_border );
}

void RenderOverlaySystem::render_bomb_overlay( int bomb_count, int radius_value, sf::Vector2f pos )
{

  auto sprite_metatype = "ICONS";
  auto position = sf::FloatRect{ pos, kGridSquareSizePixelsF };
  auto sprite_index = 2; // bomb icon
  auto scale = sf::Vector2f( 2.f, 2.f );
  RenderSystem::safe_render_sprite( sprite_metatype, position, sprite_index, scale );

  // text - slightly offset the y-axis to center with icon
  sf::Vector2f bomb_meter_offset{ 50.f, -2.f };
  sf::Text bomb_count_text( m_font, "", 30 );
  if ( bomb_count < 0 )
    bomb_count_text.setString( " INFINITE " );
  else
    bomb_count_text.setString( " =   " + std::to_string( bomb_count ) + " x " + std::to_string( radius_value ) );
  bomb_count_text.setPosition( pos + bomb_meter_offset );
  bomb_count_text.setFillColor( sf::Color::White );
  bomb_count_text.setOutlineColor( sf::Color::Black );
  bomb_count_text.setOutlineThickness( 2.f );
  m_window.draw( bomb_count_text );
}

void RenderOverlaySystem::render_player_candles_overlay( unsigned int candle_count, sf::Vector2f pos )
{
  auto sprite_metatype = "ICONS";
  auto position = sf::FloatRect{ pos, kGridSquareSizePixelsF };
  auto sprite_index = 3; // candle icon
  auto scale = sf::Vector2f( 2.f, 2.f );
  RenderSystem::safe_render_sprite( sprite_metatype, position, sprite_index, scale );

  // text - slightly offset the y-axis to center with icon
  sf::Vector2f score_meter_offset{ 50.f, -2.f };
  sf::Text player_score_text( m_font, "", 30 );
  player_score_text.setString( " =   " + std::to_string( candle_count ) );
  player_score_text.setPosition( pos + score_meter_offset );
  player_score_text.setFillColor( sf::Color::White );
  player_score_text.setOutlineColor( sf::Color::Black );
  player_score_text.setOutlineThickness( 2.f );
  m_window.draw( player_score_text );
}

void RenderOverlaySystem::render_key_count_overlay( unsigned int key_count, sf::Vector2f pos )
{
  auto sprite_metatype = "ICONS";
  auto position = sf::FloatRect{ pos, kGridSquareSizePixelsF };
  auto sprite_index = 4; // key icon
  auto scale = sf::Vector2f( 2.f, 2.f );
  RenderSystem::safe_render_sprite( sprite_metatype, position, sprite_index, scale );

  // text - slightly offset the y-axis to center with icon
  sf::Vector2f score_meter_offset{ 50.f, -2.f };
  sf::Text player_score_text( m_font, "", 30 );
  player_score_text.setString( " =   " + std::to_string( key_count ) );
  player_score_text.setPosition( pos + score_meter_offset );
  player_score_text.setFillColor( sf::Color::White );
  player_score_text.setOutlineColor( sf::Color::Black );
  player_score_text.setOutlineThickness( 2.f );
  m_window.draw( player_score_text );
}

void RenderOverlaySystem::render_relic_count_overlay( unsigned int relic_count, sf::Vector2f pos )
{
  auto sprite_metatype = "ICONS";
  auto position = sf::FloatRect{ pos, kGridSquareSizePixelsF };
  auto sprite_index = 5; // artifact icon
  auto scale = sf::Vector2f( 2.f, 2.f );
  RenderSystem::safe_render_sprite( sprite_metatype, position, sprite_index, scale );

  // text - slightly offset the y-axis to center with icon
  sf::Vector2f score_meter_offset{ 50.f, -2.f };
  sf::Text player_score_text( m_font, "", 30 );
  player_score_text.setString( " =   " + std::to_string( relic_count ) );
  player_score_text.setPosition( pos + score_meter_offset );
  player_score_text.setFillColor( sf::Color::White );
  player_score_text.setOutlineColor( sf::Color::Black );
  player_score_text.setOutlineThickness( 2.f );
  m_window.draw( player_score_text );
}

void RenderOverlaySystem::render_water_level_meter_overlay( float water_level, sf::Vector2f pos, sf::Vector2f size )
{
  // text
  waterlvl_meter_text.setPosition( pos );
  waterlvl_meter_text.setFillColor( sf::Color::White );
  waterlvl_meter_text.setOutlineColor( sf::Color::Black );
  waterlvl_meter_text.setOutlineThickness( 2.f );
  m_window.draw( waterlvl_meter_text );

  // bar fill
  sf::Vector2f waterlvl_meter_offset{ 100.f, 10.f };
  // water meter level is represented as a percentage (0-100) of the screen
  // display y-axis note: {0,0} is top left so we need to invert the Y
  // position
  float meter_meter_level = size.x - ( ( size.x / kDisplaySize.y ) * water_level );
  auto waterlvlbar = sf::RectangleShape( { meter_meter_level, size.y } );
  waterlvlbar.setPosition( pos + waterlvl_meter_offset );
  waterlvlbar.setFillColor( sf::Color::Blue );
  m_window.draw( waterlvlbar );

  // bar outline
  auto waterlvlbar_border = sf::RectangleShape( size );
  waterlvlbar_border.setPosition( pos + waterlvl_meter_offset );
  waterlvlbar_border.setFillColor( sf::Color::Transparent );
  waterlvlbar_border.setOutlineColor( sf::Color::Black );
  waterlvlbar_border.setOutlineThickness( 5.f );
  m_window.draw( waterlvlbar_border );
}

void RenderOverlaySystem::render_player_position_overlay( sf::Vector2f player_pos, sf::Vector2f pos )
{
  // text
  player_position_text.setString( "Player Position: [ " + std::to_string( static_cast<int>( player_pos.x ) ) + " , " +
                                  std::to_string( static_cast<int>( player_pos.y ) ) + " ]" );
  player_position_text.setPosition( pos );
  player_position_text.setFillColor( sf::Color::White );
  player_position_text.setOutlineColor( sf::Color::Black );
  player_position_text.setOutlineThickness( 2.f );
  m_window.draw( player_position_text );
}

void RenderOverlaySystem::render_mouse_position_overlay( sf::Vector2f mouse_position, sf::Vector2f pos )
{
  // text
  sf::Text mouse_position_text( m_font, "", 30 );
  mouse_position_text.setString( "Mouse Position: [ " + std::to_string( static_cast<int>( mouse_position.x ) ) + " , " +
                                 std::to_string( static_cast<int>( mouse_position.y ) ) + " ]" );
  mouse_position_text.setPosition( pos );
  mouse_position_text.setFillColor( sf::Color::White );
  mouse_position_text.setOutlineColor( sf::Color::Black );
  mouse_position_text.setOutlineThickness( 2.f );
  m_window.draw( mouse_position_text );
}

void RenderOverlaySystem::render_stats_overlay( sf::Vector2f pos1, sf::Vector2f pos2, sf::Vector2f pos3 )
{
  auto entity_count = m_reg->view<entt::entity>().size();
  auto npc_count = m_reg->view<Cmp::NPC>().size();
  auto position_count = m_reg->view<Cmp::Position>().size();
  auto corruption_count = m_reg->view<Cmp::CorruptionCell>().size();
  auto sinkhole_count = m_reg->view<Cmp::SinkholeCell>().size();

  auto obst_view = m_reg->view<Cmp::Obstacle>();
  auto obstacle_count = obst_view.size();
  int disabled_count = 0;
  for ( auto [e, obstacle] : obst_view.each() )
  {
    if ( not obstacle.m_enabled ) { ++disabled_count; }
  }

  sf::Text stats_text( m_font, "", 30 );
  // clang-format off
  stats_text.setString( 
       "E: " + std::to_string( entity_count ) + 
    "   P: " + std::to_string( position_count ) );
  // clang-format on
  stats_text.setPosition( pos1 );
  stats_text.setFillColor( sf::Color::White );
  stats_text.setOutlineColor( sf::Color::Black );
  stats_text.setOutlineThickness( 2.f );
  m_window.draw( stats_text );

  // clang-format off
  stats_text.setString( 
     "O: "         + std::to_string( obstacle_count ) + 
    " (disabled: " + std::to_string( disabled_count ) + ")" );
  // clang-format on
  stats_text.setPosition( pos2 );
  stats_text.setFillColor( sf::Color::White );
  stats_text.setOutlineColor( sf::Color::Black );
  stats_text.setOutlineThickness( 2.f );
  m_window.draw( stats_text );

  // clang-format off
  stats_text.setString( 
       "N: " + std::to_string( npc_count ) +
    "   C: " + std::to_string( corruption_count ) + 
    "   S: " + std::to_string( sinkhole_count ) );
  // clang-format on
  stats_text.setPosition( pos3 );
  stats_text.setFillColor( sf::Color::White );
  stats_text.setOutlineColor( sf::Color::Black );
  stats_text.setOutlineThickness( 2.f );
  m_window.draw( stats_text );
}

} // namespace ProceduralMaze::Sys