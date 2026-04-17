#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Direction.hpp>
#include <Components/Hazard/CorruptionCell.hpp>
#include <Components/Hazard/HazardFieldCell.hpp>
#include <Components/Hazard/SinkholeCell.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Constants.hpp>
#include <Inventory/FlashUICadaver.hpp>
#include <Inventory/FlashUIInventory.hpp>
#include <Inventory/FlashUIRadius.hpp>
#include <Inventory/FlashUIWealth.hpp>
#include <PathFinding/AStar.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Player.hpp>
#include <Player/PlayerBlastRadius.hpp>
#include <Player/PlayerCadaverCount.hpp>
#include <Player/PlayerHealth.hpp>
#include <Player/PlayerLevelDepth.hpp>
#include <Player/PlayerWealth.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <Shop/ShopInventory.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Utils/Optimizations.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include <iomanip>
#include <sstream>
#include <string>

namespace ProceduralMaze::Sys
{

void RenderOverlaySystem::render_ui_outlines()
{
  if ( not m_main_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw Status Outline overlay" );
    return;
  }
  for ( const auto &outline : m_main_ui_data->m_outlines )
  {
    auto rect = sf::RectangleShape( outline.rect.size );
    rect.setPosition( outline.rect.position );
    rect.setFillColor( sf::Color( 48, 48, 64, 128 ) );
    rect.setOutlineColor( sf::Color::Black );
    rect.setOutlineThickness( 5.f );
    m_window.draw( rect );
  }
}

void RenderOverlaySystem::render_ui_icons()
{
  if ( not m_main_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw icon overlay" );
    return;
  }
  for ( const auto &icon : m_main_ui_data->m_icons )
  {
    // we handle the inventory icon seperately from the others
    if ( icon.name == "inventory_icon" ) continue;
    RenderSystem::safe_render_sprite( icon.type, icon.rect, icon.index, { static_cast<float>( icon.scale ), static_cast<float>( icon.scale ) } );
  }
}

void RenderOverlaySystem::render_ui_inventory_icon()
{
  if ( not m_main_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw icon overlay" );
    return;
  }
  for ( const auto &icon : m_main_ui_data->m_icons )
  {

    if ( icon.name != "inventory_icon" ) continue;

    auto inventory_view = reg().view<Cmp::PlayerInventorySlot, Cmp::SpriteAnimation>();
    for ( auto [inventory_entt, inventory_cmp, anim_cmp] : inventory_view.each() )
    {
      RenderSystem::safe_render_sprite( anim_cmp.m_sprite_type, { icon.rect.position, Constants::kGridSizePxF }, 0,
                                        { static_cast<float>( icon.scale ), static_cast<float>( icon.scale ) } );
    }
  }
}

void RenderOverlaySystem::render_ui_meters()
{
  if ( not m_main_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw icon overlay" );
    return;
  }
  for ( const auto &meter : m_main_ui_data->m_meters )
  {
    float meter_value = 0;
    sf::Color meter_inner_color;
    bool should_render = false;

    if ( meter.name == "health_meter" )
    {
      meter_value = static_cast<float>( Utils::Player::get_health( reg() ).health );
      meter_inner_color = sf::Color::Red;
      should_render = true;
    }
    else if ( meter.name == "inventory_meter" )
    {
      meter_value = Utils::Player::get_inventory_wear_level( reg() );
      if ( meter_value >= 0 ) { should_render = true; }
      meter_inner_color = sf::Color::Red;
    }

    if ( not should_render ) { continue; }

    auto innermeter = sf::RectangleShape( { ( ( meter.rect.size.x / 100 ) * meter_value ), meter.rect.size.y } );
    innermeter.setPosition( meter.rect.position );
    innermeter.setFillColor( meter_inner_color );
    m_window.draw( innermeter );

    auto outermeter = sf::RectangleShape( meter.rect.size );
    outermeter.setPosition( meter.rect.position );
    outermeter.setFillColor( sf::Color::Transparent );
    outermeter.setOutlineColor( sf::Color::Black );
    outermeter.setOutlineThickness( 5.f );
    m_window.draw( outermeter );
  }
}

void RenderOverlaySystem::render_ui_labels( [[maybe_unused]] sf::Time dt )
{
  if ( not m_main_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw value overlay" );
    return;
  }
  for ( const auto &ui_label : m_main_ui_data->m_labels )
  {
    std::string text_str;
    if ( ui_label.name == "radius_label" ) { text_str = " =   " + std::to_string( Utils::Player::get_blast_radius( reg() ).value ); }
    else if ( ui_label.name == "cadaver_label" ) { text_str = " =   " + std::to_string( Utils::Player::get_cadaver_count( reg() ).get_count() ); }
    else if ( ui_label.name == "wealth_label" ) { text_str = " =   " + std::to_string( Utils::Player::get_wealth( reg() ).wealth ); }
    else if ( ui_label.name == "inventory_label" )
    {
      auto [entt, type] = Utils::Player::get_inventory_type( reg() );
      text_str = type;
    }

    sf::Text text( m_font, "", ui_label.font_size );
    text.setString( text_str );
    text.setPosition( ui_label.rect.position );
    text.setFillColor( sf::Color::White );
    text.setOutlineColor( sf::Color::Black );
    text.setOutlineThickness( 2.f );

    // flash the text if we just increased the bomb blast radius
    auto radius_flash_view = reg().view<Cmp::FlashUIRadius>();
    if ( ui_label.name == "radius_label" and not radius_flash_view.empty() )
    {
      auto flash_entt = radius_flash_view.front();
      auto &flash_cmp = radius_flash_view.get<Cmp::FlashUIRadius>( flash_entt );
      m_flash_radius_ui_interval += dt;
      if ( m_flash_radius_ui_interval > flash_cmp.duration )
      {
        reg().remove<Cmp::FlashUIRadius>( flash_entt );
        m_flash_radius_ui_interval = sf::Time::Zero;
      }
      else if ( static_cast<int>( m_flash_radius_ui_interval.asMilliseconds() / m_ui_flash_factor ) % 2 == 1 )
      {
        text.setFillColor( sf::Color::White );
        text.setOutlineColor( sf::Color::White );
      }
    }

    // flash the text if we just picked up a cadaver
    auto cadaver_flash_view = reg().view<Cmp::FlashUICadaver>();
    if ( ui_label.name == "cadaver_label" and not cadaver_flash_view.empty() )
    {
      auto flash_entt = cadaver_flash_view.front();
      auto &flash_cmp = cadaver_flash_view.get<Cmp::FlashUICadaver>( flash_entt );
      m_flash_cadaver_ui_interval += dt;
      if ( m_flash_cadaver_ui_interval > flash_cmp.duration )
      {
        reg().remove<Cmp::FlashUICadaver>( flash_entt );
        m_flash_cadaver_ui_interval = sf::Time::Zero;
      }
      else if ( static_cast<int>( m_flash_cadaver_ui_interval.asMilliseconds() / m_ui_flash_factor ) % 2 == 1 )
      {
        text.setFillColor( sf::Color::White );
        text.setOutlineColor( sf::Color::White );
      }
    }

    // flash the text if we just deposited something in a well
    auto wealth_flash_view = reg().view<Cmp::FlashUIWealth>();
    if ( ui_label.name == "wealth_label" and not wealth_flash_view.empty() )
    {
      auto flash_entt = wealth_flash_view.front();
      auto &flash_cmp = wealth_flash_view.get<Cmp::FlashUIWealth>( flash_entt );
      m_flash_wealth_ui_interval += dt;
      if ( m_flash_wealth_ui_interval > flash_cmp.duration )
      {
        reg().remove<Cmp::FlashUIWealth>( flash_entt );
        m_flash_wealth_ui_interval = sf::Time::Zero;
      }
      else if ( static_cast<int>( m_flash_wealth_ui_interval.asMilliseconds() / m_ui_flash_factor ) % 2 == 1 )
      {
        text.setFillColor( sf::Color::White );
        text.setOutlineColor( sf::Color::White );
      }
    }

    // flash the text if we just picked up a Key
    auto inventory_flash_view = reg().view<Cmp::FlashUIInventory>();
    if ( ui_label.name == "inventory_label" and not inventory_flash_view.empty() )
    {
      auto flash_entt = inventory_flash_view.front();
      auto &flash_cmp = inventory_flash_view.get<Cmp::FlashUIInventory>( flash_entt );
      m_flash_inventory_ui_interval += dt;
      if ( m_flash_inventory_ui_interval > flash_cmp.duration )
      {
        reg().remove<Cmp::FlashUIInventory>( flash_entt );
        m_flash_inventory_ui_interval = sf::Time::Zero;
      }
      else if ( static_cast<int>( m_flash_inventory_ui_interval.asMilliseconds() / m_ui_flash_factor ) % 2 == 1 )
      {
        text.setFillColor( sf::Color::Black );
        text.setOutlineColor( sf::Color::White );
      }
    }

    m_window.draw( text );
  }
}

void RenderOverlaySystem::render_level_depth()
{
  auto player_level_cmp = Utils::Player::get_level_depth( reg() );
  if ( player_level_cmp.display_timer.getElapsedTime() >= player_level_cmp.display_cooldown ) { return; }

  auto display_res = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg() );
  sf::Text level_txt( m_font, "Nekropolis " + std::to_string( player_level_cmp.get_count() ), 100 );
  level_txt.setPosition( { ( display_res.x / 2.f ) - level_txt.getLocalBounds().getCenter().x, display_res.y / 2.f } );
  level_txt.setFillColor( sf::Color::Blue );
  m_window.draw( level_txt );
}

void RenderOverlaySystem::render_shop_inventory_overlay()
{
  auto inventory_view = reg().view<Cmp::ShopInventory>().each();
  for ( auto [inventory_entt, inventory_cmp] : inventory_view )
  {
    if ( not inventory_cmp.is_enabled ) continue;

    auto game_view = m_window.getView();

    auto [_, inventory_ui_pos] = inventory_cmp.m_config.get_position();
    auto [_, inventory_ui_size] = inventory_cmp.m_config.get_size();

    // Convert world-space positions to screen pixels
    auto to_screen = [&]( sf::Vector2f world_pos ) -> sf::Vector2f { return sf::Vector2f( m_window.mapCoordsToPixel( world_pos, game_view ) ); };

    // Convert world-space size to screen-space size
    // (size is relative so we compute it as a delta from origin)
    sf::Vector2f screen_origin = to_screen( inventory_ui_pos );
    sf::Vector2f screen_corner = to_screen( inventory_ui_pos + inventory_ui_size );
    sf::Vector2f screen_size = screen_corner - screen_origin;

    m_window.setView( m_window.getDefaultView() );

    sf::RectangleShape border;
    border.setSize( screen_size );
    border.setPosition( screen_origin );
    border.setFillColor( inventory_cmp.m_config.ui_mainbgcolor );
    border.setOutlineColor( inventory_cmp.m_config.ui_mainlinecolor );
    border.setOutlineThickness( inventory_cmp.m_config.ui_mainlinesize );
    m_window.draw( border );

    auto current_slot_pos = inventory_ui_pos;
    current_slot_pos.y += Constants::kGridSizePxF.y;

    for ( auto [i, slot] : std::views::enumerate( inventory_cmp.m_slots ) )
    {
      auto &[item, price] = slot;

      current_slot_pos.x += inventory_cmp.m_config.slot_padding;

      sf::Vector2f screen_slot_pos = to_screen( current_slot_pos );
      sf::Vector2f screen_slot_corner = to_screen( current_slot_pos + Constants::kGridSizePxF );
      sf::Vector2f screen_slot_size = screen_slot_corner - screen_slot_pos;
      float slot_center_x = screen_slot_pos.x + ( screen_slot_size.x / 2.f );

      sf::Text slot_idx_txt( m_font, std::to_string( i + 1 ), inventory_cmp.m_config.ui_fontsize + 10 );
      slot_idx_txt.setFillColor( inventory_cmp.m_config.ui_fontcolor );
      float idx_txt_x = slot_center_x - ( slot_idx_txt.getLocalBounds().size.x / 2.f );
      if ( i == 0 ) idx_txt_x -= slot_idx_txt.getLocalBounds().position.x; // Correct offset for narrow glyphs like '1'
      slot_idx_txt.setPosition( { idx_txt_x, static_cast<float>( screen_slot_pos.y - slot_idx_txt.getCharacterSize() * 1.5 ) } );
      m_window.draw( slot_idx_txt );

      sf::RectangleShape slot_rect;
      slot_rect.setSize( screen_slot_size );
      slot_rect.setPosition( screen_slot_pos );
      slot_rect.setFillColor( inventory_cmp.m_config.ui_slotbgcolor );
      slot_rect.setOutlineColor( inventory_cmp.m_config.ui_slotlinecolor );
      slot_rect.setOutlineThickness( inventory_cmp.m_config.ui_slotlinesize );
      m_window.draw( slot_rect );

      sf::Text slot_desc_txt( m_font, m_sprite_factory.get_display_name_by_type( item ), inventory_cmp.m_config.ui_fontsize );
      slot_desc_txt.setFillColor( inventory_cmp.m_config.ui_fontcolor );
      float desc_txt_x = slot_center_x - ( slot_desc_txt.getLocalBounds().size.x / 2.f );
      slot_desc_txt.setPosition( { desc_txt_x, screen_slot_pos.y + screen_slot_size.y } );
      m_window.draw( slot_desc_txt );

      sf::Text slot_price_txt( m_font, std::to_string( price ), inventory_cmp.m_config.ui_fontsize );
      slot_price_txt.setFillColor( inventory_cmp.m_config.ui_fontcolor );
      float price_txt_x = slot_center_x - ( slot_price_txt.getLocalBounds().size.x / 2.f );
      slot_price_txt.setPosition( { price_txt_x, screen_slot_pos.y + screen_slot_size.y + slot_price_txt.getCharacterSize() * 2 } );
      m_window.draw( slot_price_txt );

      // Sprites need world-space view
      m_window.setView( game_view );
      RenderSystem::safe_render_sprite( item, { current_slot_pos, Constants::kGridSizePxF }, 0, sf::Vector2f{ 1, 1 } );
      m_window.setView( m_window.getDefaultView() );

      current_slot_pos.x += Constants::kGridSizePxF.x;
    }

    m_window.setView( game_view );
  }
}

void RenderOverlaySystem::render_player_position_overlay()
{
  if ( not m_dbg_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw player position overlay" );
    return;
  }
  auto player_pos = Utils::Player::get_position( reg() );

  for ( const auto &ui_label : m_dbg_ui_data->m_labels )
  {
    if ( ui_label.name != "player_position" ) { continue; }
    sf::Text text( m_font, "Player Pos:", ui_label.font_size );
    text.setString( "Player Position: [ " + std::to_string( static_cast<int>( player_pos.x() ) ) + " , " +
                    std::to_string( static_cast<int>( player_pos.y() ) ) + " ]" );
    text.setPosition( ui_label.rect.position );
    text.setFillColor( sf::Color::White );
    text.setOutlineColor( sf::Color::Black );
    text.setOutlineThickness( 2.f );
    m_window.draw( text );
  }
}

void RenderOverlaySystem::render_mouse_position_overlay()
{

  if ( not m_dbg_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw mouse position overlay" );
    return;
  }

  sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( m_window );
  sf::Vector2f mouse_world_pos = m_window.mapPixelToCoords( mouse_pixel_pos, RenderSystem::getGameView() );

  for ( const auto &ui_label : m_dbg_ui_data->m_labels )
  {
    if ( ui_label.name != "mouse_position" ) { continue; }

    sf::Text text( m_font, "Mouse Pos:", ui_label.font_size );
    text.setString( "Mouse Position: [ " + std::to_string( static_cast<int>( mouse_world_pos.x ) ) + " , " +
                    std::to_string( static_cast<int>( mouse_world_pos.y ) ) + " ]" );

    text.setPosition( ui_label.rect.position );
    text.setFillColor( sf::Color::White );
    text.setOutlineColor( sf::Color::Black );
    text.setOutlineThickness( 2.f );
    m_window.draw( text );
  }
}

void RenderOverlaySystem::render_stats_overlay()
{
  if ( not m_dbg_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw entity stats overlay" );
    return;
  }

  for ( const auto &ui_label : m_dbg_ui_data->m_labels )
  {
    if ( ui_label.name != "entity_stats" ) { continue; }
    auto entity_count = reg().view<entt::entity>().size();
    auto npc_count = reg().view<Cmp::NPC>().size();
    auto position_count = reg().view<Cmp::Position>().size();
    auto corruption_count = reg().view<Cmp::CorruptionCell>().size();
    auto sinkhole_count = reg().view<Cmp::SinkholeCell>().size();
    auto crypt_passage_block_count = reg().view<Cmp::CryptPassageBlock>().size();

    auto obst_view = reg().view<Cmp::Obstacle>();
    auto obstacle_count = obst_view.size();

    // clang-format off
    sf::Text text1(m_font, "", ui_label.font_size);
    text1.setString( 
      "E: " + std::to_string( entity_count ) + 
      "   P: " + std::to_string( position_count ) +
      "   O: " + std::to_string( obstacle_count ));

    sf::Text text2(m_font, "", ui_label.font_size);
    text2.setString( 
      "CPB: " + std::to_string(crypt_passage_block_count) +
      "   N: " + std::to_string( npc_count ) +
      "   C: " + std::to_string( corruption_count ) + 
      "   S: " + std::to_string( sinkhole_count ) );
    // clang-format on

    text1.setPosition( ui_label.rect.position );
    text1.setFillColor( sf::Color::White );
    text1.setOutlineColor( sf::Color::Black );
    text1.setOutlineThickness( 2.f );
    m_window.draw( text1 );

    text2.setPosition( { ui_label.rect.position.x, ui_label.rect.position.y + 40 } );
    text2.setFillColor( sf::Color::White );
    text2.setOutlineColor( sf::Color::Black );
    text2.setOutlineThickness( 2.f );
    m_window.draw( text2 );
  }
}

void RenderOverlaySystem::render_zorder_values_overlay( std::vector<ZOrder> &zorder_queue )
{

  if ( not m_dbg_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw zorder list overlay" );
    return;
  }

  std::set<Sprites::SpriteMetaType> exclusions = { "ROCK",
                                                   //  "CRYPT.interior_sb",
                                                   "WALL", "PLAYERSPAWN", "NPCSKELE", "NPCGHOST", "DETONATED", "FOOTSTEPS", "HOLYWELL.interior_wall",
                                                   "RUIN.interior_wall", "CRYPT.interior_wall" };

  for ( const auto &ui_label : m_dbg_ui_data->m_labels )
  {
    if ( ui_label.name != "zorder_list" ) { continue; }

    float count = 0;
    for ( const auto &zorder_entry : zorder_queue )
    {
      if ( reg().all_of<Cmp::SpriteAnimation>( zorder_entry.e ) )
      {
        auto &sprite_anim_cmp = reg().get<Cmp::SpriteAnimation>( zorder_entry.e );
        if ( exclusions.find( sprite_anim_cmp.m_sprite_type ) != exclusions.end() ) { continue; }

        std::stringstream ss;
        ss << "z: " << std::fixed << std::setprecision( 1 ) << zorder_entry.z << " | ";
        ss << "e: " << static_cast<uint32_t>( zorder_entry.e ) << " | ";
        ss << sprite_anim_cmp.m_sprite_type << " | ";
        sf::Text m_z_text( m_font, ss.str(), ui_label.font_size );
        m_z_text.setFillColor( sf::Color::White );
        m_z_text.setPosition( { ui_label.rect.position.x, ui_label.rect.position.y + count } );
        m_z_text.setOutlineColor( sf::Color::Black );
        m_z_text.setOutlineThickness( 0.5f );
        m_window.draw( m_z_text );
        count += ui_label.font_size; // Move down for the next entry
      }
    }
  }
}

void RenderOverlaySystem::render_npc_list_overlay()
{
  if ( not m_dbg_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw zorder list overlay" );
    return;
  }
  for ( const auto &ui_label : m_dbg_ui_data->m_labels )
  {
    if ( ui_label.name != "npc_list" ) { continue; }

    auto npc_view = reg().view<Cmp::NPC, Cmp::Position, Cmp::SpriteAnimation>();
    m_npc_list_text.clear();
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp, npc_anim_cmp] : npc_view.each() )
    {
      sf::Text npc_text( m_font, "", ui_label.font_size );

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

    // Render in sorted order (map automatically sorts by key)
    int count = 0;
    for ( auto &[key, npc_text] : m_npc_list_text )
    {
      npc_text.setPosition( ui_label.rect.position + sf::Vector2f{ 0, count * 20.f } );
      m_window.draw( npc_text );
      ++count;
    }
  }
}

void RenderOverlaySystem::render_obstacle_markers()
{
  if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
  {
    auto obstacle_view = reg().view<Cmp::Position, Cmp::Obstacle>();
    for ( auto [ob_entt, pos_cmp, obst_cmp] : obstacle_view.each() )
    {
      sf::RectangleShape obstacle_shape( Constants::kGridSizePxF );
      obstacle_shape.setPosition( pos_cmp.position );
      obstacle_shape.setOutlineThickness( 1.f );
      m_window.draw( obstacle_shape );
    }
  }
}

void RenderOverlaySystem::render_lerp_positions()
{
  auto lerp_view = reg().view<Cmp::LerpPosition, Cmp::Direction, Cmp::NPC, Cmp::Position>();
  for ( auto [entity, lerp_pos_cmp, dir_cmp, npc_cmp, npc_pos_cmp] : lerp_view.each() )
  {
    sf::RectangleShape lerp_start_pos_rect( Constants::kGridSizePxF );
    lerp_start_pos_rect.setPosition( lerp_pos_cmp.m_start );
    lerp_start_pos_rect.setFillColor( sf::Color::Transparent );
    lerp_start_pos_rect.setOutlineColor( sf::Color::Yellow );
    lerp_start_pos_rect.setOutlineThickness( 1.f );
    m_window.draw( lerp_start_pos_rect );

    sf::RectangleShape lerp_stop_pos_rect( Constants::kGridSizePxF );
    lerp_stop_pos_rect.setPosition( lerp_pos_cmp.m_target );
    lerp_stop_pos_rect.setFillColor( sf::Color::Transparent );
    lerp_stop_pos_rect.setOutlineColor( sf::Color::Cyan );
    lerp_stop_pos_rect.setOutlineThickness( 1.f );
    m_window.draw( lerp_stop_pos_rect );
  }
}

void RenderOverlaySystem::render_spatial_grid_neighbours( const Cmp::Position &query_pos, sf::Color color, PathFinding::QueryCompass query_compass )
{
  if ( PathFinding::SpatialHashGridSharedPtr spatialgrid_ptr = m_pathfinding_navmesh.lock() )
  {
    std::vector<entt::entity> neighbours_list = spatialgrid_ptr->neighbours( Cmp::Position( query_pos.position, query_pos.size ), query_compass );
    for ( auto neighbour_entt : neighbours_list )
    {
      auto *neighbour_pos = reg().try_get<Cmp::Position>( neighbour_entt );
      if ( not neighbour_pos ) continue;
      if ( reg().any_of<Cmp::PlayerCharacter, Cmp::NPC>( neighbour_entt ) ) continue;

      sf::RectangleShape rectangle;
      rectangle.setSize( neighbour_pos->size );
      rectangle.setPosition( neighbour_pos->position );
      rectangle.setFillColor( sf::Color::Transparent );
      rectangle.setOutlineThickness( 1.f );
      rectangle.setOutlineColor( color );
      m_window.draw( rectangle );
    }
  }
}

void RenderOverlaySystem::render_pathfinding_vector( const Cmp::Position &start_pos_cmp, const Cmp::Position &end_pos_cmp, sf::Color color,
                                                     PathFinding::QueryCompass query_compass )
{
  if ( not Utils::is_visible_in_view( RenderSystem::getGameView(), start_pos_cmp ) ) return;

  if ( PathFinding::SpatialHashGridSharedPtr spatialgrid_ptr = m_pathfinding_navmesh.lock() )
  {
    std::vector<PathFinding::PathNode> path = PathFinding::astar( reg(), *spatialgrid_ptr, start_pos_cmp, end_pos_cmp, query_compass );

    for ( auto pathnode : path )
    {
      auto expand_lever_pos_hitbox = Cmp::RectBounds::scaled( pathnode.pos.position, pathnode.pos.size, 0.2f );
      sf::RectangleShape rectangle;
      rectangle.setSize( expand_lever_pos_hitbox.size() );
      rectangle.setPosition( expand_lever_pos_hitbox.position() );
      rectangle.setFillColor( sf::Color::Transparent );
      rectangle.setOutlineColor( color );
      rectangle.setOutlineThickness( 1.f );
      m_window.draw( rectangle );
    }
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

void RenderOverlaySystem::render_wear_level( float wearlevel, const Cmp::Position &pos )
{

  float icon_border = 0.f;
  float padding = 1.f;
  float icon_height = 2.f;
  float icon_width = Constants::kGridSizePxF.x - ( padding * 2 );

  sf::RectangleShape icon( { ( icon_width / 100.f ) * wearlevel, icon_height } );
  icon.setOutlineColor( sf::Color::Black );
  icon.setOutlineThickness( icon_border );
  icon.setFillColor( sf::Color( 255, 0, 0, 224 ) );

  icon.setPosition( { pos.position.x + ( padding ), pos.position.y + Constants::kGridSizePxF.y - icon_height - ( padding ) } );

  m_window.draw( icon );
}

} // namespace ProceduralMaze::Sys