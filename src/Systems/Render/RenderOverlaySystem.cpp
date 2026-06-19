#include <Components/AnimData.hpp>
#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Direction.hpp>
#include <Components/Hazard/CorruptionCell.hpp>
#include <Components/Hazard/HazardFieldCell.hpp>
#include <Components/Hazard/SinkholeCell.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/Inventory/PlayerInventorySlot.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/Player/PlayerBlastRadius.hpp>
#include <Components/Player/PlayerCadaverCount.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerLevelDepth.hpp>
#include <Components/Player/PlayerWealth.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Shop/ShopInventory.hpp>
#include <Components/ZOrderValue.hpp>
#include <Crypt/CryptPassageDoor.hpp>
#include <Crypt/CryptRoomLavaPit.hpp>
#include <Crypt/CryptRoomLavaPitCell.hpp>
#include <Exit.hpp>
#include <FootStepTimer.hpp>
#include <Inventory/FlashUICadaver.hpp>
#include <Inventory/FlashUIInventory.hpp>
#include <Inventory/FlashUIRadius.hpp>
#include <Inventory/FlashUIWealth.hpp>
#include <Inventory/Grimoire.hpp>
#include <LastDirection.hpp>
#include <Moveable.hpp>
#include <Npc/NpcNoPathFinding.hpp>
#include <PathFinding/AStar.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Player/PlayerNoPath.hpp>
#include <ReservedPosition.hpp>
#include <Ruin/RuinCobweb.hpp>
#include <SceneControl/Scenes/CryptScene.hpp>
#include <SelectedPosition.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Stores/ItemStore.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>

#include <VoidPosition.hpp>
#include <Wall.hpp>
#include <iomanip>
#include <ranges>
#include <sstream>
#include <string>

namespace Game::Sys
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
    rect.setFillColor( outline.fill_color );
    rect.setOutlineColor( outline.line_color );
    rect.setOutlineThickness( static_cast<float>( outline.line_thickness ) );
    draw_screen( rect );
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
    RenderSystem::safe_render_sprite_screen( icon.type, icon.rect, icon.index,
                                             { static_cast<float>( icon.scale ), static_cast<float>( icon.scale ) } );
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

    auto inventory_view = reg().view<Cmp::PlayerInventorySlot, Cmp::AnimData>();
    for ( auto [inventory_entt, inventory_cmp, anim_cmp] : inventory_view.each() )
    {
      RenderSystem::safe_render_sprite_screen( anim_cmp.m_sprite_type, { icon.rect.position, Constants::kGridSizePxF }, 0,
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
      meter_value = static_cast<float>( Utils::Player::get_player_stats( reg() ).health() );
      meter_inner_color = sf::Color::Red;
      should_render = true;
    }
    else if ( meter.name == "fear_meter" )
    {
      meter_value = static_cast<float>( Utils::Player::get_player_stats( reg() ).fear() );
      meter_inner_color = sf::Color::Blue;
      should_render = true;
    }
    else if ( meter.name == "infamy_meter" )
    {
      meter_value = static_cast<float>( Utils::Player::get_player_stats( reg() ).infamy() );
      meter_inner_color = sf::Color::Green;
      should_render = true;
    }
    else if ( meter.name == "despair_meter" )
    {
      meter_value = static_cast<float>( Utils::Player::get_player_stats( reg() ).despair() );
      meter_inner_color = sf::Color::Yellow;
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
    draw_screen( innermeter );

    auto outermeter = sf::RectangleShape( meter.rect.size );
    outermeter.setPosition( meter.rect.position );
    outermeter.setFillColor( sf::Color::Transparent );
    outermeter.setOutlineColor( sf::Color::Black );
    outermeter.setOutlineThickness( 5.f );
    draw_screen( outermeter );
  }
}

void RenderOverlaySystem::render_ui_texts()
{
  if ( not m_main_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw value overlay" );
    return;
  }

  for ( const auto &ui_text : m_main_ui_data->m_texts )
  {
    sf::Text text( m_font, ui_text.value, ui_text.font_size );
    text.setPosition( ui_text.rect.position );
    text.setFillColor( sf::Color::White );
    text.setOutlineColor( sf::Color::Black );
    text.setOutlineThickness( 2.f );
    draw_screen( text );
  }
}

void RenderOverlaySystem::render_ui_labels( sf::Time dt )
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
      text_str = m_sprite_factory.get_spritesheet_by_type( type ).get_display_name();
      if ( text_str == "Error Sprite" ) { text_str = ""; }
    }

    sf::Text text( m_font, "", ui_label.font_size );
    text.setString( text_str );
    if ( ui_label.align == "center" )
    {
      text.setPosition( { ui_label.rect.position.x - text.getLocalBounds().getCenter().x, ui_label.rect.position.y } );
    }
    else if ( ui_label.align == "left" ) { text.setPosition( ui_label.rect.position ); }
    else if ( ui_label.align == "right" )
    {
      text.setPosition( { ui_label.rect.position.x - text.getLocalBounds().size.x, ui_label.rect.position.y } );
    }
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

    draw_screen( text );
  }
}

void RenderOverlaySystem::render_level_depth()
{
  auto player_level_cmp = Utils::Player::get_level_depth( reg() );
  if ( player_level_cmp.display_timer.getElapsedTime() >= player_level_cmp.display_cooldown ) { return; }

  auto display_res = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg() );
  sf::Text level_txt( m_font, "Nekropolis " + std::to_string( player_level_cmp.get_count() ), 100 );
  level_txt.setPosition( sf::Vector2f( ( static_cast<float>( display_res.x ) / 2.f ) - level_txt.getLocalBounds().getCenter().x, 100.f ) );
  level_txt.setFillColor( sf::Color::White );
  draw_screen( level_txt );
}

void RenderOverlaySystem::render_shop_inventory_overlay()
{

  // only draw the shop if its enabled
  auto inventory_view = reg().view<Cmp::ShopInventory>();
  if ( inventory_view.empty() ) { return; }
  auto &inventory_cmp = inventory_view.get<Cmp::ShopInventory>( inventory_view.front() );
  if ( not inventory_cmp.is_enabled ) return;

  if ( not m_shop_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw value overlay" );
    return;
  }

  // Draw all UI outlines
  for ( const auto &outline : m_shop_ui_data->m_outlines )
  {
    auto rect = sf::RectangleShape( outline.rect.size );
    rect.setPosition( outline.rect.position );
    rect.setFillColor( outline.fill_color );
    rect.setOutlineColor( outline.line_color );
    rect.setOutlineThickness( static_cast<float>( outline.line_thickness ) );
    draw_screen( rect );
  }

  // Draw all UI Icons
  for ( auto [icon, slot] : std::views::zip( m_shop_ui_data->m_icons, inventory_cmp.m_slots ) )
  {
    auto &[item, price] = slot;
    RenderSystem::safe_render_sprite_screen( Sys::ItemStore::instance().get_item( item ).sprite_type, { icon.rect.position, Constants::kGridSizePxF },
                                             0, sf::Vector2f{ static_cast<float>( icon.scale ), static_cast<float>( icon.scale ) } );
  }

  //! @brief Helper to draw predefined `sf_text` at `pos`
  auto draw_label_text_at = [&]( sf::Text &sf_text, const sf::Vector2f &pos, const std::string &align = "left" )
  {
    if ( align == "center" ) { sf_text.setPosition( { pos.x - sf_text.getLocalBounds().getCenter().x, pos.y } ); }
    else if ( align == "left" ) { sf_text.setPosition( pos ); }
    else if ( align == "right" ) { sf_text.setPosition( { pos.x - sf_text.getLocalBounds().size.x, pos.y } ); }

    draw_screen( sf_text );
  };

  // Draw all text labels (index, description, price)
  for ( auto [i, slot] : std::views::enumerate( inventory_cmp.m_slots ) )
  {
    auto &[item, price] = slot;

    sf::Text slot_idx_txt( m_font, std::to_string( i + 1 ), 30 );
    slot_idx_txt.setFillColor( sf::Color::Black );

    Sprites::SpriteMetaType sprite_mtype = Sys::ItemStore::instance().get_item( item ).sprite_type;
    sf::Text slot_desc_txt( m_font, m_sprite_factory.get_display_name_by_type( sprite_mtype ), 30 );
    slot_desc_txt.setFillColor( sf::Color::Black );

    sf::Text slot_price_txt( m_font, std::to_string( price ), 30 );
    slot_price_txt.setFillColor( sf::Color::Black );

    for ( const auto &ui_label : m_shop_ui_data->m_labels )
    {
      switch ( i )
      {
        case 0:
          if ( ui_label.name.contains( "slot1_idx" ) ) { draw_label_text_at( slot_idx_txt, ui_label.rect.position, ui_label.align ); }
          if ( ui_label.name.contains( "slot1_desc" ) ) { draw_label_text_at( slot_desc_txt, ui_label.rect.position, ui_label.align ); }
          if ( ui_label.name.contains( "slot1_price" ) ) { draw_label_text_at( slot_price_txt, ui_label.rect.position, ui_label.align ); }

          break;
        case 1:
          if ( ui_label.name.contains( "slot2_idx" ) ) { draw_label_text_at( slot_idx_txt, ui_label.rect.position, ui_label.align ); }
          if ( ui_label.name.contains( "slot2_desc" ) ) { draw_label_text_at( slot_desc_txt, ui_label.rect.position, ui_label.align ); }
          if ( ui_label.name.contains( "slot2_price" ) ) { draw_label_text_at( slot_price_txt, ui_label.rect.position, ui_label.align ); }
          break;

        case 2:
          if ( ui_label.name.contains( "slot3_idx" ) ) { draw_label_text_at( slot_idx_txt, ui_label.rect.position, ui_label.align ); }
          if ( ui_label.name.contains( "slot3_desc" ) ) { draw_label_text_at( slot_desc_txt, ui_label.rect.position, ui_label.align ); }
          if ( ui_label.name.contains( "slot3_price" ) ) { draw_label_text_at( slot_price_txt, ui_label.rect.position, ui_label.align ); }
          break;

        default:
          break;
      }
    }
  }
}

void RenderOverlaySystem::render_grimoire_inventory_overlay()
{
  // only draw the shop if its enabled
  auto grimoire_view = reg().view<Cmp::Grimoire>();
  if ( grimoire_view.empty() ) { return; }
  auto &grimoire_cmp = grimoire_view.get<Cmp::Grimoire>( grimoire_view.front() );
  if ( not grimoire_cmp.is_enabled ) return;

  sf::Vector2f rect_position( 600.f, 400.f );
  auto rect = sf::RectangleShape( rect_position );
  rect.setPosition( { 500.f, 300.f } );
  rect.setFillColor( sf::Color::Black );
  rect.setOutlineColor( sf::Color::White );
  rect.setOutlineThickness( 2.f );
  draw_screen( rect );

  float y_offset = 0.f;
  constexpr unsigned int font_size = 18;
  constexpr float line_height = 22.f;

  //! @brief Draw a text line in the UI
  auto draw_line = [&]( const std::string &str, sf::Color color = sf::Color::White )
  {
    sf::Text text( m_font, str, font_size );
    text.setFillColor( color );
    text.setOutlineColor( sf::Color::Black );
    text.setOutlineThickness( 1.f );
    text.setPosition( { rect_position.x, rect_position.y + y_offset } );
    draw_screen( text );
    y_offset += line_height;
  };

  // Draw all UI outlines
  for ( const auto &[item, is_enabled] : grimoire_cmp.contents )
  {
    draw_line( item + " - " + ( is_enabled ? "Shown" : "Hidden" ) );
  }
}

void RenderOverlaySystem::render_ui_misc_stats()
{
  if ( not m_dbg_ui_data ) { return; }

  float y_offset = 0.f;
  constexpr unsigned int font_size = 18;
  constexpr float line_height = 22.f;
  for ( const auto &ui_label : m_dbg_ui_data->m_labels )
  {
    if ( ui_label.name != "entity_stats" ) { continue; }

    //! @brief Draw a text line in the UI
    auto draw_line = [&]( const std::string &str, sf::Color color = sf::Color::White )
    {
      sf::Text text( m_font, str, font_size );
      text.setFillColor( color );
      text.setOutlineColor( sf::Color::Black );
      text.setOutlineThickness( 1.f );
      text.setPosition( { ui_label.rect.position.x, ui_label.rect.position.y + y_offset } );
      draw_screen( text );
      y_offset += line_height;
    };

    draw_line( "--- Stats ---", sf::Color::Yellow );

    draw_line( " Player position - [" + std::to_string( static_cast<int>( Utils::Player::get_position( reg() ).x() ) ) + " , " +
               std::to_string( static_cast<int>( Utils::Player::get_position( reg() ).y() ) ) + "]" );

    sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( m_window );
    sf::Vector2f mouse_world_pos = m_window.mapPixelToCoords( mouse_pixel_pos, RenderSystem::get_world_view() );
    draw_line( " Mouse position - [" + std::to_string( static_cast<int>( mouse_world_pos.x ) ) + " , " +
               std::to_string( static_cast<int>( mouse_world_pos.y ) ) + "]" );

    draw_line( " Player current direction - [" + std::to_string( Utils::Player::get_direction( reg() ).x ) + " , " +
               std::to_string( Utils::Player::get_direction( reg() ).y ) + "]" );
    draw_line( " Player last direction - [" + std::to_string( Utils::Player::get_last_direction( reg() ).x ) + " , " +
               std::to_string( Utils::Player::get_last_direction( reg() ).y ) + "]" );

    draw_line( " Entities - " + std::to_string( reg().view<entt::entity>().size() ) );
    draw_line( " NPCs - " + std::to_string( reg().view<Cmp::NPC>().size() ) );
    draw_line( " Corruption - " + std::to_string( reg().view<Cmp::CorruptionCell>().size() ) );
    draw_line( " Sinkhole - " + std::to_string( reg().view<Cmp::CorruptionCell>().size() ) );
    draw_line( " CryptPassageBlocks - " + std::to_string( reg().view<Cmp::CryptPassageBlock>().size() ) );
  }
}

void RenderOverlaySystem::render_ui_zorder_list( std::vector<ZOrder> &zorder_queue )
{

  if ( not m_dbg_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw zorder list overlay" );
    return;
  }

  // clang-format off
  std::set<Sprites::SpriteMetaType> exclusions = {
      "sprite.graveyard.wall.int.main",
      "sprite.graveyard.wall.int.cap",
      "sprite.ruin.wall.int",
      "sprite.well.wall.int",
      "sprite.shop.wall.int",
      "sprite.ruin.wall.ext", 
      "sprite.crypt.wall.ext",
      "sprite.graveyard.playerspawn", 
      "sprite.skeleton",      
      "sprite.ghost",
      "sprite.wisp",
      "sprite.graveyard.detonated",
      "sprite.player.footsteps",
      "sprite.ruin.cobweb",
      "sprite.ruin.bookcase.left",
      "sprite.ruin.bookcase.mid",
      "sprite.ruin.bookcase.right"
  };
  // clang-format on

  float y_offset = 0.f;
  constexpr unsigned int font_size = 18;
  constexpr float line_height = 22.f;

  for ( const auto &ui_label : m_dbg_ui_data->m_labels )
  {
    if ( ui_label.name != "zorder_list" ) { continue; }

    //! @brief Draw a text line in the UI
    auto draw_line = [&]( const std::string &str, sf::Color color = sf::Color::White )
    {
      sf::Text text( m_font, str, font_size );
      text.setFillColor( color );
      text.setOutlineColor( sf::Color::Black );
      text.setOutlineThickness( 1.f );
      text.setPosition( { ui_label.rect.position.x, ui_label.rect.position.y + y_offset } );
      draw_screen( text );
      y_offset += line_height;
    };

    draw_line( "--- Z Order List ---", sf::Color::Yellow );

    // float count = 0;
    for ( const auto &zorder_entry : zorder_queue )
    {

      std::string name;
      auto *sprite_anim_cmp = reg().try_get<Cmp::AnimData>( zorder_entry.e );
      if ( sprite_anim_cmp )
      {
        if ( exclusions.find( sprite_anim_cmp->m_sprite_type ) != exclusions.end() ) { continue; }
        name = sprite_anim_cmp->m_sprite_type;
      }

      auto *particle_sprite_owner = reg().try_get<ParticleSpriteOwner>( zorder_entry.e );
      if ( particle_sprite_owner )
      {
        if ( exclusions.find( particle_sprite_owner->sprite->get_tag() ) != exclusions.end() ) { continue; }
        name = particle_sprite_owner->sprite->get_tag();
      }

      if ( name.empty() ) continue;
      // clang-format off
      draw_line(" Z:" + std::to_string(static_cast<int>(zorder_entry.z)) + 
                " E:" + std::to_string(static_cast<uint32_t>( zorder_entry.e )) +
                " - " + name);
      // clang-format on
    }
  }
}

void RenderOverlaySystem::render_ui_npc_list()
{
  if ( not m_dbg_ui_data )
  {
    SPDLOG_CRITICAL( "UiData object is not initialised. Cannot draw zorder list overlay" );
    return;
  }

  float y_offset = 0.f;
  constexpr unsigned int font_size = 18;
  constexpr float line_height = 22.f;

  for ( const auto &ui_label : m_dbg_ui_data->m_labels )
  {
    if ( ui_label.name != "npc_list" ) { continue; }

    //! @brief Draw a text line in the UI
    auto draw_line = [&]( const std::string &str, sf::Color color = sf::Color::White )
    {
      sf::Text text( m_font, str, font_size );
      text.setFillColor( color );
      text.setOutlineColor( sf::Color::Black );
      text.setOutlineThickness( 1.f );
      text.setPosition( { ui_label.rect.position.x, ui_label.rect.position.y + y_offset } );
      draw_screen( text );
      y_offset += line_height;
    };

    draw_line( "--- NPCs ---", sf::Color::Yellow );

    auto npc_view = reg().view<Cmp::NPC, Cmp::Position, Cmp::AnimData>();
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp, npc_anim_cmp] : npc_view.each() )
    {
      // clang-format off
      draw_line( " " + std::to_string( entt::to_integral( npc_entity ) ) +
                 ": [" + std::to_string( static_cast<int>( npc_pos_cmp.position.x ) ) + 
                 "," + std::to_string( static_cast<int>( npc_pos_cmp.position.x ) ) + "] - " + 
                 npc_anim_cmp.m_sprite_type);
      // clang-format on
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
    draw_world( lerp_start_pos_rect );

    sf::RectangleShape lerp_stop_pos_rect( Constants::kGridSizePxF );
    lerp_stop_pos_rect.setPosition( lerp_pos_cmp.m_target );
    lerp_stop_pos_rect.setFillColor( sf::Color::Transparent );
    lerp_stop_pos_rect.setOutlineColor( sf::Color::Cyan );
    lerp_stop_pos_rect.setOutlineThickness( 1.f );
    draw_world( lerp_stop_pos_rect );
  }
}

void RenderOverlaySystem::render_square( sf::Vector2f pos, sf::Vector2f size, sf::Color color )
{
  sf::RectangleShape rect( size );
  rect.setPosition( pos );
  rect.setFillColor( sf::Color::Transparent );
  rect.setOutlineColor( color );
  rect.setOutlineThickness( 1.f );
  draw_world( rect );
}

void RenderOverlaySystem::render_spatial_grid_neighbours( const Cmp::Position &query_pos, sf::Color color, PathFinding::QueryCompass query_compass )
{
  if ( PathFinding::SpatialHashGridSharedPtr spatialgrid_ptr = m_npc_navmesh.lock() )
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
      draw_world( rectangle );
    }
  }
}

void RenderOverlaySystem::render_pathfinding_vector( const Cmp::Position &start_pos_cmp, const Cmp::Position &end_pos_cmp, sf::Color color,
                                                     PathFinding::QueryCompass query_compass )
{
  if ( not Utils::is_visible_in_view( RenderSystem::get_world_view(), start_pos_cmp ) ) return;

  if ( PathFinding::SpatialHashGridSharedPtr spatialgrid_ptr = m_npc_navmesh.lock() )
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
      draw_world( rectangle );
    }
  }
}

void RenderOverlaySystem::render_ui_entity_inspect()
{
  if ( not m_dbg_ui_data ) { return; }

  sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( m_window );
  sf::Vector2f mouse_world_pos = m_window.mapPixelToCoords( mouse_pixel_pos, RenderSystem::get_world_view() );

  float y_offset = 0.f;
  constexpr unsigned int font_size = 18;
  constexpr float line_height = 22.f;
  for ( const auto &ui_label : m_dbg_ui_data->m_labels )
  {
    if ( ui_label.name != "inspect_list" ) { continue; }

    auto position_view = reg().view<Cmp::Position>();
    for ( auto [entity, pos_cmp] : position_view.each() )
    {
      if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), pos_cmp ) ) continue;
      if ( not pos_cmp.contains( mouse_world_pos ) ) { continue; }

      // Header: entity ID
      auto draw_line = [&]( const std::string &str, sf::Color color = sf::Color::White )
      {
        sf::Text text( m_font, str, font_size );
        text.setFillColor( color );
        text.setOutlineColor( sf::Color::Black );
        text.setOutlineThickness( 1.f );
        text.setPosition( { ui_label.rect.position.x, ui_label.rect.position.y + y_offset } );
        draw_screen( text );
        y_offset += line_height;
      };

      draw_line( "--- Entity #" + std::to_string( entt::to_integral( entity ) ) + " ---", sf::Color::Yellow );

      if ( auto *cmp = reg().try_get<Cmp::AnimData>( entity ) )
      {
        auto sprite_idx = std::to_string( cmp->getFrameIndexOffset() );
        draw_line( " " + cmp->m_sprite_type + " [" + sprite_idx + "]" );
      }
      if ( reg().all_of<Cmp::VoidPosition>( entity ) ) draw_line( "  Void", sf::Color::White );
      if ( reg().all_of<Cmp::ReservedPosition>( entity ) ) draw_line( "  ReservedPosition", sf::Color::Red );
      if ( reg().all_of<Cmp::NpcNoPathFinding>( entity ) ) draw_line( "  NpcNoPathFinding", sf::Color::Red );
      if ( reg().all_of<Cmp::PlayerNoPath>( entity ) ) draw_line( " PlayerNoPath", sf::Color::Red );
      if ( reg().all_of<Cmp::Moveable>( entity ) ) draw_line( " Moveable", sf::Color::Green );
      if ( reg().all_of<Cmp::SelectedPosition>( entity ) ) draw_line( " Selected", sf::Color::Green );
      if ( reg().all_of<Cmp::Exit>( entity ) ) draw_line( " Exit", sf::Color::Green );
      if ( reg().all_of<Cmp::Obstacle>( entity ) ) draw_line( " Obstacle", sf::Color::Green );
      if ( reg().all_of<Cmp::CryptPassageBlock>( entity ) ) draw_line( " PassageBlock", sf::Color::Cyan );
      if ( reg().all_of<Cmp::FootStepTimer>( entity ) ) draw_line( " Footsteps", sf::Color::Cyan );
      if ( reg().all_of<Cmp::RuinCobweb>( entity ) ) draw_line( " Cobweb", sf::Color::Cyan );
      if ( reg().all_of<Cmp::Wall>( entity ) ) draw_line( " Wall", sf::Color::Cyan );
      if ( reg().all_of<Cmp::CryptPassageDoor>( entity ) ) draw_line( " PassageDoor", sf::Color::Green );
      if ( reg().all_of<Cmp::CryptRoomLavaPitCell>( entity ) ) draw_line( " CryptRoomLavaPitCell", sf::Color{ 255, 165, 0 } );
      if ( reg().all_of<Cmp::CryptRoomLavaPit>( entity ) ) draw_line( " CryptRoomLavaPit", sf::Color{ 255, 165, 0 } );

      if ( auto *cmp = reg().try_get<Cmp::UUID>( entity ) ) draw_line( " " + cmp->str(), sf::Color::White );

      auto posx = std::to_string( static_cast<int>( pos_cmp.position.x ) );
      auto posy = std::to_string( static_cast<int>( pos_cmp.position.y ) );
      draw_line( "  Pos: [ " + posx + " , " += posy + " ]" );

      auto sizex = std::to_string( static_cast<int>( pos_cmp.size.y ) );
      auto sizey = std::to_string( static_cast<int>( pos_cmp.size.y ) );
      draw_line( "  Size: [ " + sizex + " , " += sizey + " ]" );

      if ( auto *cmp = reg().try_get<Cmp::ZOrderValue>( entity ) )
      {
        auto zorder = std::to_string( cmp->getZOrder() );
        draw_line( "  ZOrder: " + zorder );
      }
    }
  }
}

void RenderOverlaySystem::render_crypt_maze_timer( sf::Vector2f pos, unsigned int size )
{
  auto &clock = Scene::CryptScene::get_maze_timer();
  if ( clock.isRunning() )
  {
    sf::Text clock_text( m_font, "", size );
    std::stringstream ss;
    ss << std::fixed << std::setprecision( 1 ) << 10.f - clock.getElapsedTime().asSeconds();
    clock_text.setString( ss.str() );
    clock_text.setPosition( { pos.x - ( clock_text.getLocalBounds().size.x / 2 ), pos.y } );
    clock_text.setFillColor( sf::Color::Red );
    clock_text.setOutlineColor( sf::Color::Black );
    clock_text.setOutlineThickness( 2.f );
    draw_screen( clock_text );
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

  draw_world( icon );
}

} // namespace Game::Sys