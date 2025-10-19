#include <RenderOverlaySystem.hpp>
#include <SFML/Graphics/Text.hpp>

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
      getWindow().draw( distance_text );
    }
    entt_distance_set++;
  }
}

void RenderOverlaySystem::render_bomb_radius_overlay( int radius_value, sf::Vector2f pos )
{
  // text
  bomb_radius_text.setPosition( pos );
  bomb_radius_text.setFillColor( sf::Color::White );
  bomb_radius_text.setOutlineColor( sf::Color::Black );
  bomb_radius_text.setOutlineThickness( 2.f );
  getWindow().draw( bomb_radius_text );

  // text
  sf::Text bomb_radius_value_text( m_font, "", 30 );
  bomb_radius_value_text.setString( std::to_string( radius_value ) );
  bomb_radius_value_text.setPosition( pos + sf::Vector2f( 180.f, 0.f ) );
  bomb_radius_value_text.setFillColor( sf::Color::White );
  bomb_radius_value_text.setOutlineColor( sf::Color::Black );
  bomb_radius_value_text.setOutlineThickness( 2.f );
  getWindow().draw( bomb_radius_value_text );
}

void RenderOverlaySystem::render_bomb_overlay( int bomb_count, sf::Vector2f pos )
{
  // text
  bomb_inventory_text.setPosition( pos );
  bomb_inventory_text.setFillColor( sf::Color::White );
  bomb_inventory_text.setOutlineColor( sf::Color::Black );
  bomb_inventory_text.setOutlineThickness( 2.f );
  getWindow().draw( bomb_inventory_text );

  // text
  sf::Text bomb_count_text( m_font, "", 30 );
  if ( bomb_count < 0 )
    bomb_count_text.setString( " INFINITE " );
  else
    bomb_count_text.setString( " x " + std::to_string( bomb_count ) );
  bomb_count_text.setPosition( pos + sf::Vector2f( 100.f, 0.f ) );
  bomb_count_text.setFillColor( sf::Color::White );
  bomb_count_text.setOutlineColor( sf::Color::Black );
  bomb_count_text.setOutlineThickness( 2.f );
  getWindow().draw( bomb_count_text );
}

void RenderOverlaySystem::render_health_overlay( float health_value, sf::Vector2f pos, sf::Vector2f size )
{
  // text
  healthlvl_meter_text.setPosition( pos );
  healthlvl_meter_text.setFillColor( sf::Color::White );
  healthlvl_meter_text.setOutlineColor( sf::Color::Black );
  healthlvl_meter_text.setOutlineThickness( 2.f );
  getWindow().draw( healthlvl_meter_text );

  // bar fill
  sf::Vector2f healthbar_offset{ 100.f, 10.f };
  auto healthbar = sf::RectangleShape( { ( ( size.x / 100 ) * health_value ), size.y } );
  healthbar.setPosition( pos + healthbar_offset );
  healthbar.setFillColor( sf::Color::Red );
  getWindow().draw( healthbar );

  // bar outline
  auto healthbar_border = sf::RectangleShape( size );
  healthbar_border.setPosition( pos + healthbar_offset );
  healthbar_border.setFillColor( sf::Color::Transparent );
  healthbar_border.setOutlineColor( sf::Color::Black );
  healthbar_border.setOutlineThickness( 5.f );
  getWindow().draw( healthbar_border );
}

void RenderOverlaySystem::render_water_level_meter_overlay( float water_level, sf::Vector2f pos, sf::Vector2f size )
{
  // text
  waterlvl_meter_text.setPosition( pos );
  waterlvl_meter_text.setFillColor( sf::Color::White );
  waterlvl_meter_text.setOutlineColor( sf::Color::Black );
  waterlvl_meter_text.setOutlineThickness( 2.f );
  getWindow().draw( waterlvl_meter_text );

  // bar fill
  sf::Vector2f waterlvl_meter_offset{ 100.f, 10.f };
  // water meter level is represented as a percentage (0-100) of the screen
  // display y-axis note: {0,0} is top left so we need to invert the Y
  // position
  float meter_meter_level = size.x - ( ( size.x / kDisplaySize.y ) * water_level );
  auto waterlvlbar = sf::RectangleShape( { meter_meter_level, size.y } );
  waterlvlbar.setPosition( pos + waterlvl_meter_offset );
  waterlvlbar.setFillColor( sf::Color::Blue );
  getWindow().draw( waterlvlbar );

  // bar outline
  auto waterlvlbar_border = sf::RectangleShape( size );
  waterlvlbar_border.setPosition( pos + waterlvl_meter_offset );
  waterlvlbar_border.setFillColor( sf::Color::Transparent );
  waterlvlbar_border.setOutlineColor( sf::Color::Black );
  waterlvlbar_border.setOutlineThickness( 5.f );
  getWindow().draw( waterlvlbar_border );
}

void RenderOverlaySystem::render_player_position_overlay( sf::Vector2f player_pos, sf::Vector2f pos )
{
  // text
  std::stringstream ss;
  ss << "Player Position: [ " << floor( player_pos.x ) << " , " << floor( player_pos.y ) << " ]";

  sf::Text player_position_text( m_font, ss.str(), 30 );
  player_position_text.setPosition( pos );
  player_position_text.setFillColor( sf::Color::White );
  player_position_text.setOutlineColor( sf::Color::Black );
  player_position_text.setOutlineThickness( 2.f );
  getWindow().draw( player_position_text );
}

} // namespace ProceduralMaze::Sys