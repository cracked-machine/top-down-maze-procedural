#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Direction.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcScanBounds.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Player/PlayerDetectionBounds.hpp>
#include <Components/Player/PlayerDistance.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <Components/Hazard/CorruptionCell.hpp>
#include <Components/Hazard/HazardFieldCell.hpp>
#include <Components/Hazard/SinkholeCell.hpp>
#include <Components/Position.hpp>
#include <SFML/System/Vector2.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Utils/Utils.hpp>

#include <iomanip>
#include <sstream>
#include <string>

namespace ProceduralMaze::Sys
{

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
  auto position = sf::FloatRect{ pos, Constants::kGridSquareSizePixelsF };
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
  auto position = sf::FloatRect{ pos, Constants::kGridSquareSizePixelsF };
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

void RenderOverlaySystem::render_bomb_overlay( int radius_value, sf::Vector2f pos )
{

  auto sprite_metatype = "ICONS";
  auto position = sf::FloatRect{ pos, Constants::kGridSquareSizePixelsF };
  auto sprite_index = 2; // bomb icon
  auto scale = sf::Vector2f( 2.f, 2.f );
  RenderSystem::safe_render_sprite( sprite_metatype, position, sprite_index, scale );

  // text - slightly offset the y-axis to center with icon
  sf::Vector2f bomb_meter_offset{ 50.f, -2.f };
  sf::Text bomb_count_text( m_font, "", 30 );
  bomb_count_text.setString( std::to_string( radius_value ) );
  bomb_count_text.setPosition( pos + bomb_meter_offset );
  bomb_count_text.setFillColor( sf::Color::White );
  bomb_count_text.setOutlineColor( sf::Color::Black );
  bomb_count_text.setOutlineThickness( 2.f );
  m_window.draw( bomb_count_text );
}

void RenderOverlaySystem::render_cadaver_count_overlay( unsigned int cadaver_count, sf::Vector2f pos )
{
  auto sprite_metatype = "ICONS";
  auto position = sf::FloatRect{ pos, Constants::kGridSquareSizePixelsF };
  auto sprite_index = 6; // cadaver icon
  auto scale = sf::Vector2f( 2.f, 2.f );
  RenderSystem::safe_render_sprite( sprite_metatype, position, sprite_index, scale );

  // text - slightly offset the y-axis to center with icon
  sf::Vector2f score_meter_offset{ 50.f, -2.f };
  sf::Text player_score_text( m_font, "", 30 );
  player_score_text.setString( " =   " + std::to_string( cadaver_count ) );
  player_score_text.setPosition( pos + score_meter_offset );
  player_score_text.setFillColor( sf::Color::White );
  player_score_text.setOutlineColor( sf::Color::Black );
  player_score_text.setOutlineThickness( 2.f );
  m_window.draw( player_score_text );
}

void RenderOverlaySystem::render_wealth_overlay( unsigned int wealth_value, sf::Vector2f pos )
{
  auto sprite_metatype = "ICONS";
  auto position = sf::FloatRect{ pos, Constants::kGridSquareSizePixelsF };
  auto sprite_index = 7; // gold coins icon
  auto scale = sf::Vector2f( 2.f, 2.f );
  RenderSystem::safe_render_sprite( sprite_metatype, position, sprite_index, scale );

  // text - slightly offset the y-axis to center with icon
  sf::Vector2f score_meter_offset{ 50.f, -2.f };
  sf::Text player_score_text( m_font, "", 30 );
  player_score_text.setString( " =   " + std::to_string( wealth_value ) );
  player_score_text.setPosition( pos + score_meter_offset );
  player_score_text.setFillColor( sf::Color::White );
  player_score_text.setOutlineColor( sf::Color::Black );
  player_score_text.setOutlineThickness( 2.f );
  m_window.draw( player_score_text );
}

void RenderOverlaySystem::render_inventory_overlay( sf::Vector2f pos )
{
  float ui_padding = 5.f;
  float icon_scale = 5.f;
  sf::Vector2f ui_pos( pos );
  sf::Vector2f ui_size( ( Constants::kGridSquareSizePixelsF * icon_scale ) + ( sf::Vector2f( ui_padding, ui_padding ) ) );
  sf::Vector2f icon_pos( pos.x + ui_padding, pos.y + ui_padding );

  auto ui_background = sf::RectangleShape( ui_size );
  ui_background.setPosition( ui_pos );
  ui_background.setFillColor( sf::Color( 48, 48, 64, 128 ) );
  m_window.draw( ui_background );

  auto ui_edge = sf::RectangleShape( ui_size );
  ui_edge.setPosition( ui_pos );
  ui_edge.setFillColor( sf::Color( sf::Color::Transparent ) );
  ui_edge.setOutlineColor( sf::Color::Black );
  ui_edge.setOutlineThickness( 5.f );
  m_window.draw( ui_edge );

  auto inventory_view = getReg().view<Cmp::PlayerInventorySlot, Cmp::SpriteAnimation>();
  for ( auto [inventory_entt, inventory_cmp, anim_cmp] : inventory_view.each() )
  {
    RenderSystem::safe_render_sprite( anim_cmp.m_sprite_type, { icon_pos, Constants::kGridSquareSizePixelsF }, 0,
                                      sf::Vector2f{ icon_scale, icon_scale } );
    sf::Text display_name( m_font, m_sprite_factory.get_display_name_by_type( inventory_cmp.type ), 30 );
    // Calculate the center of the scaled icon
    float icon_center_x = icon_pos.x + ( Constants::kGridSquareSizePixelsF.x * icon_scale ) / 2.f;
    // Center the text horizontally relative to the icon center
    float text_x = icon_center_x - display_name.getLocalBounds().size.x / 2.f;
    display_name.setPosition( sf::Vector2f{ text_x, icon_pos.y + ui_size.y + ui_padding } );

    display_name.setFillColor( sf::Color::White );
    m_window.draw( display_name );
  }
}

void RenderOverlaySystem::render_water_level_meter_overlay( float water_level, sf::Vector2f pos, sf::Vector2f size )
{
  // text
  m_waterlvl_meter_text.setPosition( pos );
  m_waterlvl_meter_text.setFillColor( sf::Color::White );
  m_waterlvl_meter_text.setOutlineColor( sf::Color::Black );
  m_waterlvl_meter_text.setOutlineThickness( 2.f );
  m_window.draw( m_waterlvl_meter_text );

  // bar fill
  sf::Vector2f waterlvl_meter_offset{ 100.f, 10.f };
  // water meter level is represented as a percentage (0-100) of the screen
  // display y-axis note: {0,0} is top left so we need to invert the Y
  // position
  sf::Vector2u display_size = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DisplayResolution>( getReg() );
  float meter_meter_level = size.x - ( ( size.x / display_size.y ) * water_level );
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
  if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
  {
    m_player_position_text.setString( "Player Position: [ " + std::to_string( static_cast<int>( player_pos.x ) ) + " , " +
                                      std::to_string( static_cast<int>( player_pos.y ) ) + " ]" );
  }
  m_player_position_text.setPosition( pos );
  m_player_position_text.setFillColor( sf::Color::White );
  m_player_position_text.setOutlineColor( sf::Color::Black );
  m_player_position_text.setOutlineThickness( 2.f );
  m_window.draw( m_player_position_text );
}

void RenderOverlaySystem::render_mouse_position_overlay( sf::Vector2f mouse_position, sf::Vector2f pos )
{
  // text
  if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
  {

    m_mouse_position_text.setString( "Mouse Position: [ " + std::to_string( static_cast<int>( mouse_position.x ) ) + " , " +
                                     std::to_string( static_cast<int>( mouse_position.y ) ) + " ]" );
  }
  m_mouse_position_text.setPosition( pos );
  m_mouse_position_text.setFillColor( sf::Color::White );
  m_mouse_position_text.setOutlineColor( sf::Color::Black );
  m_mouse_position_text.setOutlineThickness( 2.f );
  m_window.draw( m_mouse_position_text );
}

void RenderOverlaySystem::render_stats_overlay( sf::Vector2f pos1, sf::Vector2f pos2 )
{
  // only gather stats every interval
  if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
  {
    auto entity_count = getReg().view<entt::entity>().size();
    auto npc_count = getReg().view<Cmp::NPC>().size();
    auto position_count = getReg().view<Cmp::Position>().size();
    auto corruption_count = getReg().view<Cmp::CorruptionCell>().size();
    auto sinkhole_count = getReg().view<Cmp::SinkholeCell>().size();
    auto crypt_passage_block_count = getReg().view<Cmp::CryptPassageBlock>().size();

    auto obst_view = getReg().view<Cmp::Obstacle>();
    auto obstacle_count = obst_view.size();

    // clang-format off
    m_stats_text1.setString( 
      "E: " + std::to_string( entity_count ) + 
      "   P: " + std::to_string( position_count ) +
      "   O: " + std::to_string( obstacle_count ));
    m_stats_text2.setString( 
      "CPB: " + std::to_string(crypt_passage_block_count) +
      "   N: " + std::to_string( npc_count ) +
      "   C: " + std::to_string( corruption_count ) + 
      "   S: " + std::to_string( sinkhole_count ) );
    // clang-format on

    m_stats_text1.setPosition( pos1 );
    m_stats_text1.setFillColor( sf::Color::White );
    m_stats_text1.setOutlineColor( sf::Color::Black );
    m_stats_text1.setOutlineThickness( 2.f );
    m_window.draw( m_stats_text1 );

    m_stats_text2.setPosition( pos2 );
    m_stats_text2.setFillColor( sf::Color::White );
    m_stats_text2.setOutlineColor( sf::Color::Black );
    m_stats_text2.setOutlineThickness( 2.f );
    m_window.draw( m_stats_text2 );
  }
}

void RenderOverlaySystem::render_zorder_values_overlay( sf::Vector2f pos, std::vector<ZOrder> &zorder_queue,
                                                        std::set<Sprites::SpriteMetaType> exclusions )
{
  uint32_t font_size = 15;
  float count = 0;
  for ( const auto &zorder_entry : zorder_queue )
  {
    if ( getReg().all_of<Cmp::SpriteAnimation>( zorder_entry.e ) )
    {
      auto &sprite_anim_cmp = getReg().get<Cmp::SpriteAnimation>( zorder_entry.e );
      if ( exclusions.find( sprite_anim_cmp.m_sprite_type ) != exclusions.end() ) { continue; }

      std::stringstream ss;
      ss << "z: " << std::fixed << std::setprecision( 1 ) << zorder_entry.z << " | ";
      ss << "e: " << static_cast<uint32_t>( zorder_entry.e ) << " | ";
      ss << sprite_anim_cmp.m_sprite_type << " | ";
      sf::Text m_z_text{ m_font, ss.str(), font_size };
      m_z_text.setFillColor( sf::Color::White );
      m_z_text.setPosition( { pos.x, pos.y + count } );
      m_z_text.setOutlineColor( sf::Color::Black );
      m_z_text.setOutlineThickness( 0.5f );
      m_window.draw( m_z_text );
      count += font_size; // Move down for the next entry
    }
  }
}

void RenderOverlaySystem::render_npc_list_overlay( sf::Vector2f text_start_pos )
{
  // refresh the list only at intervals
  if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
  {
    auto npc_view = getReg().view<Cmp::NPC, Cmp::Position, Cmp::SpriteAnimation>();
    m_npc_list_text.clear();
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp, npc_anim_cmp] : npc_view.each() )
    {
      sf::Text npc_text( m_font, "", 20 );

      // Pad the type name to consistent width
      std::string type_str = npc_anim_cmp.m_sprite_type;
      int padding = 20;
      type_str.resize( padding, ' ' ); // Pad to 20 characters

      // Format entity ID with padding
      padding = 10;
      std::string entity_id = std::to_string( entt::to_integral( npc_entity ) );
      entity_id.insert( 0, padding - std::min( padding, (int)entity_id.length() ),
                        ' ' ); // Right-align in padding chars

      std::stringstream ss;
      // clang-format off
      ss << type_str
         << ": # " << entity_id
         << " - [ x: " << std::setw(4) << std::right << static_cast<int>( npc_pos_cmp.position.x )
         << " , y: " << std::setw(4) << std::right << static_cast<int>( npc_pos_cmp.position.y ) << " ]";
      // clang-format on
      npc_text.setString( ss.str() );
      npc_text.setFillColor( sf::Color::White );
      npc_text.setOutlineColor( sf::Color::Black );
      npc_text.setOutlineThickness( 1.f );
      m_npc_list_text.emplace( entt::to_integral( npc_entity ), std::move( npc_text ) );
    }
  }

  // Render in sorted order (map automatically sorts by key)
  int count = 0;
  for ( auto &[key, npc_text] : m_npc_list_text )
  {
    npc_text.setPosition( text_start_pos + sf::Vector2f{ 0, count * 20.f } );
    m_window.draw( npc_text );
    ++count;
  }
}

void RenderOverlaySystem::render_obstacle_markers()
{
  if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
  {
    auto obstacle_view = getReg().view<Cmp::Position, Cmp::Obstacle>();
    for ( auto [ob_entt, pos_cmp, obst_cmp] : obstacle_view.each() )
    {
      sf::RectangleShape obstacle_shape( Constants::kGridSquareSizePixelsF );
      obstacle_shape.setPosition( pos_cmp.position );
      obstacle_shape.setOutlineThickness( 1.f );
      m_window.draw( obstacle_shape );
    }
  }
}

void RenderOverlaySystem::render_player_distances()
{
  auto obstacle_view = getReg().view<Cmp::Position, Cmp::PlayerDistance>();
  for ( auto [ob_entt, pos_cmp, player_dist_cmp] : obstacle_view.each() )
  {

    m_distance_text.setString( std::to_string( player_dist_cmp.distance ) );
    m_distance_text.setPosition( pos_cmp.position + sf::Vector2f{ 5.f, 0.f } );
    m_distance_text.setFillColor( sf::Color::White );
    m_distance_text.setOutlineColor( sf::Color::Black );
    m_distance_text.setOutlineThickness( 1.f );
    m_window.draw( m_distance_text );
  }
}

void RenderOverlaySystem::render_scan_detection_bounds()
{
  auto player_view = getReg().view<Cmp::Direction, Cmp::PCDetectionBounds>();
  for ( auto [entity, pc_pos_cmp, pc_detection_bounds] : player_view.each() )
  {
    sf::RectangleShape detection_bounds_shape;
    detection_bounds_shape.setPosition( pc_detection_bounds.position() );
    detection_bounds_shape.setSize( pc_detection_bounds.size() );
    detection_bounds_shape.setFillColor( sf::Color::Transparent );
    detection_bounds_shape.setOutlineColor( sf::Color::Blue );
    detection_bounds_shape.setOutlineThickness( 1.f );
    m_window.draw( detection_bounds_shape );
  }

  for ( auto [entity, pos_cmp, npc_cmp] : getReg().view<Cmp::Position, Cmp::NPC>().each() )
  {
    auto npc_sb_cmp = getReg().try_get<Cmp::NPCScanBounds>( entity );
    if ( not npc_sb_cmp ) continue;
    sf::RectangleShape scan_bounds_shape;
    scan_bounds_shape.setPosition( npc_sb_cmp->position() );
    scan_bounds_shape.setSize( npc_sb_cmp->size() );
    scan_bounds_shape.setFillColor( sf::Color::Transparent );
    scan_bounds_shape.setOutlineColor( sf::Color::Red );
    scan_bounds_shape.setOutlineThickness( 1.f );
    m_window.draw( scan_bounds_shape );
  }
}

void RenderOverlaySystem::render_lerp_positions()
{
  auto lerp_view = getReg().view<Cmp::LerpPosition, Cmp::Direction, Cmp::NPC, Cmp::Position>();
  for ( auto [entity, lerp_pos_cmp, dir_cmp, npc_cmp, npc_pos_cmp] : lerp_view.each() )
  {
    sf::RectangleShape lerp_start_pos_rect( Constants::kGridSquareSizePixelsF );
    lerp_start_pos_rect.setPosition( lerp_pos_cmp.m_start );
    lerp_start_pos_rect.setFillColor( sf::Color::Transparent );
    lerp_start_pos_rect.setOutlineColor( sf::Color::Yellow );
    lerp_start_pos_rect.setOutlineThickness( 1.f );
    m_window.draw( lerp_start_pos_rect );

    sf::RectangleShape lerp_stop_pos_rect( Constants::kGridSquareSizePixelsF );
    lerp_stop_pos_rect.setPosition( lerp_pos_cmp.m_target );
    lerp_stop_pos_rect.setFillColor( sf::Color::Transparent );
    lerp_stop_pos_rect.setOutlineColor( sf::Color::Cyan );
    lerp_stop_pos_rect.setOutlineThickness( 1.f );
    m_window.draw( lerp_stop_pos_rect );

    // clang-format off
    auto horizontal_hitbox = 
      Cmp::RectBounds( sf::Vector2f{ npc_pos_cmp.position.x + ( dir_cmp.x * 16 ), npc_pos_cmp.position.y },
      Constants::kGridSquareSizePixelsF, 0.5f, Cmp::RectBounds::ScaleCardinality::BOTH );

    auto vertical_hitbox = 
      Cmp::RectBounds( sf::Vector2f{ npc_pos_cmp.position.x, npc_pos_cmp.position.y + ( dir_cmp.y * 16 ) },
      Constants::kGridSquareSizePixelsF, 0.5f, Cmp::RectBounds::ScaleCardinality::BOTH );
    // clang-format on

    // auto hlerp_hitbox = Cmp::RectBounds( sf::Vector2f{ lerp_pos_cmp.m_target.x - ( dir_cmp.x * 8
    // ), lerp_pos_cmp.m_target.y },
    //                                      kGridSquareSizePixelsF, 0.5f,
    //                                      Cmp::RectBounds::ScaleCardinality::BOTH );
    sf::RectangleShape lerp_diag_pos_hrect( horizontal_hitbox.size() );
    lerp_diag_pos_hrect.setPosition( horizontal_hitbox.position() );
    lerp_diag_pos_hrect.setFillColor( sf::Color::Transparent );
    lerp_diag_pos_hrect.setOutlineColor( sf::Color::Green );
    lerp_diag_pos_hrect.setOutlineThickness( 1.f );
    m_window.draw( lerp_diag_pos_hrect );

    // auto vlerp_hitbox = Cmp::RectBounds( sf::Vector2f{ lerp_pos_cmp.m_target.x,
    // lerp_pos_cmp.m_target.y - ( dir_cmp.y * 8 ) },
    //                                      kGridSquareSizePixelsF, 0.5f,
    //                                      Cmp::RectBounds::ScaleCardinality::BOTH );
    sf::RectangleShape lerp_diag_pos_vrect( vertical_hitbox.size() );
    lerp_diag_pos_vrect.setPosition( vertical_hitbox.position() );
    lerp_diag_pos_vrect.setFillColor( sf::Color::Transparent );
    lerp_diag_pos_vrect.setOutlineColor( sf::Color::Green );
    lerp_diag_pos_vrect.setOutlineThickness( 1.f );
    m_window.draw( lerp_diag_pos_vrect );
  }
}

void RenderOverlaySystem::render_crypt_maze_timer( sf::Vector2f pos, unsigned int size )
{
  if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
  {

    auto &clock = Scene::CryptScene::get_maze_timer();
    if ( clock.isRunning() )
    {
      sf::Text clock_text( m_font, "", size );
      std::stringstream ss;
      ss << std::fixed << std::setprecision( 1 ) << 10.f - clock.getElapsedTime().asSeconds();
      clock_text.setString( ss.str() );
      clock_text.setPosition( { pos.x - clock_text.getLocalBounds().size.x / 2, pos.y } );
      clock_text.setFillColor( sf::Color::Red );
      clock_text.setOutlineColor( sf::Color::Black );
      clock_text.setOutlineThickness( 2.f );
      m_window.draw( clock_text );
    }
  }
}

void RenderOverlaySystem::render_wear_level( [[maybe_unused]] Cmp::InventoryWearLevel &wearlevel, const Cmp::Position &pos )
{
  float icon_border = 0.f;
  float padding = 1.f;
  float icon_height = 2.f;
  float icon_width = Constants::kGridSquareSizePixelsF.x - ( padding * 2 );

  sf::RectangleShape icon( { ( icon_width / 100.f ) * wearlevel.m_level, icon_height } );
  icon.setOutlineColor( sf::Color::Black );
  icon.setOutlineThickness( icon_border );
  icon.setFillColor( sf::Color( 255, 0, 0, 224 ) );
  icon.setPosition( { pos.position.x + ( padding ), pos.position.y + Constants::kGridSquareSizePixelsF.y - icon_height - ( padding ) } );
  m_window.draw( icon );
}

} // namespace ProceduralMaze::Sys