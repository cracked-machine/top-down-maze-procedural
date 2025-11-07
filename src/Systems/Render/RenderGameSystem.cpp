#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Door.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/GraveSprite.hpp>
#include <Components/HazardFieldCell.hpp>
#include <Components/LargeObstacle.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/NpcDeathPosition.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/ShrineSprite.hpp>
#include <Components/SinkholeCell.hpp>
#include <Components/SpawnAreaSprite.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Components/WeaponLevel.hpp>
#include <Components/Wormhole.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

#include <string>

namespace ProceduralMaze::Sys {

RenderGameSystem::RenderGameSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                                    Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : RenderSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "RenderGameSystem initialized" );
}

void RenderGameSystem::init_views()
{
  // init local view dimensions
  m_local_view = sf::View( { kLocalMapViewSize.x * 0.5f, kLocalMapViewSize.y * 0.5f }, kLocalMapViewSize );
  m_local_view.setViewport( sf::FloatRect( { 0.f, 0.f }, { 1.f, 1.f } ) );

  // init minimap view dimensions
  m_minimap_view = sf::View( { kDisplaySize.x * 0.5f, kDisplaySize.y * 0.5f },
                             { kDisplaySize.x * kMiniMapViewZoomFactor, kDisplaySize.y * kMiniMapViewZoomFactor } );
  m_minimap_view.setViewport( sf::FloatRect( { 0.75f, 0.f }, { 0.25f, 0.25f } ) );

  auto start_pos = get_persistent_component<Cmp::Persistent::PlayerStartPosition>();
  m_local_view.setCenter( start_pos );
  m_minimap_view.setCenter( start_pos );
}

void RenderGameSystem::init_shaders()
{
  m_water_shader.setup();
  m_wormhole_shader.setup();
  m_pulsing_shader.setup();
  m_mist_shader.setup();
}

void RenderGameSystem::init_tilemap() { m_floormap.load( kMapGridSize, "res/json/tilemap_config.json" ); }

void RenderGameSystem::render_game( [[maybe_unused]] sf::Time deltaTime, RenderOverlaySystem &overlay_sys )
{
  using namespace Sprites;

  // check for updates to the System modes
  for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
  {
    m_show_path_distances = _sys.show_path_distances;
    m_show_armed_obstacles = _sys.show_armed_obstacles;
    m_minimap_enabled = _sys.minimap_enabled;
    m_show_debug_stats = _sys.show_debug_stats;
  }

  sf::FloatRect player_position( { 0.f, 0.f }, kGridSquareSizePixelsF );
  for ( auto [entity, pc_cmp, pc_pos_cmp] : m_reg->view<Cmp::PlayableCharacter, Cmp::Position>().each() )
  {
    player_position = pc_pos_cmp;
  }
  // main render begin
  m_window.clear();
  {
    // local view begin - this shows only a `LOCAL_MAP_VIEW_SIZE` of the game
    // world
    m_window.setView( m_local_view );
    {
      // update the static game view reference
      RenderSystem::s_game_view = m_local_view;
      // move the local view position to equal the player position

      m_local_view.setCenter( player_position.position );
      // draw the background

      render_flood_waters( player_position );
      render_floormap( { 0, 0 } );

      // now draw everything else on top
      render_sinkhole();
      render_corruption();
      render_wormhole();

      render_npc_containers();
      render_loot_containers();
      render_small_obstacles();

      render_armed();
      render_walls();
      render_player_spawn();
      render_player_footsteps();
      render_player();
      render_npc();
      render_large_obstacles();
      render_loot();
      render_explosions();
      render_arrow_compass();
      render_mist( player_position );

      if ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval )
      {
        render_player_distances_on_npc();
        render_player_distances_on_obstacles();
        m_debug_update_timer.restart();
      }

      // render_positions();
    }
    // local view end

    // minimap view begin - this show a quarter of the game world but in a
    // much smaller scale
    if ( m_minimap_enabled )
    {
      m_window.setView( m_minimap_view );
      {
        render_floormap( { 0, kMapGridOffset.y * BaseSystem::kGridSquareSizePixels.y } );
        render_loot_containers();
        render_small_obstacles();

        render_sinkhole();
        render_corruption();
        render_wormhole();
        render_armed();
        render_loot();
        render_walls();
        render_player_spawn();
        render_player();
        render_npc();
        render_large_obstacles();
        // render_flood_waters();

        // update the minimap view center based on player position
        // reset the center if player is stuck
        m_minimap_view.setCenter( player_position.position );
      }
    }
    // minimap view end

    // UI Overlays begin (these will always be displayed no matter where the
    // player moves)
    m_window.setView( m_window.getDefaultView() );
    {
      if ( m_minimap_enabled )
      {
        auto minimap_border = sf::RectangleShape( {
            m_window.getSize().x * kMiniMapViewZoomFactor, // 25% of screen width
            m_window.getSize().y * kMiniMapViewZoomFactor  // 25% of screen height
        } );
        minimap_border.setPosition( { m_window.getSize().x - minimap_border.getSize().x, 0.f } );

        minimap_border.setFillColor( sf::Color::Transparent );
        minimap_border.setOutlineColor( sf::Color::White );
        minimap_border.setOutlineThickness( 2.f );
        m_window.draw( minimap_border );
      }
      // init metrics
      int player_health = 0;
      int bomb_inventory = 0;
      int blast_radius = 0;
      int new_weapon_level = 0;
      int player_candles_count = 0;
      int player_keys_count = 0;
      int player_relic_count = 0;
      sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( m_window );
      sf::Vector2f mouse_world_pos = m_window.mapPixelToCoords( mouse_pixel_pos, RenderSystem::getGameView() );

      // gather metrics from components
      for ( auto [pc_entt, pc_cmp, pc_health_cmp] : m_reg->view<Cmp::PlayableCharacter, Cmp::PlayerHealth>().each() )
      {
        player_health = pc_health_cmp.health;
        bomb_inventory = pc_cmp.bomb_inventory;
        blast_radius = pc_cmp.blast_radius;
      }

      for ( auto [entity, weapon_level, pc_cmp] : m_reg->view<Cmp::WeaponLevel, Cmp::PlayableCharacter>().each() )
      {
        new_weapon_level = weapon_level.m_level;
      }

      auto pc_candles_cmp = m_reg->view<Cmp::PlayerCandlesCount, Cmp::PlayerKeysCount, Cmp::PlayerRelicCount>();
      for ( auto [entity, candles_cmp, keys_cmp, relic_cmp] : pc_candles_cmp.each() )
      {
        player_candles_count = candles_cmp.get_count();
        player_keys_count = keys_cmp.get_count();
        player_relic_count = relic_cmp.get_count();
      }

      // render metrics
      overlay_sys.render_ui_background_overlay( { 20.f, 20.f }, { 300.f, 270.f } );
      overlay_sys.render_health_overlay( player_health, { 40.f, 40.f }, { 200.f, 20.f } );
      overlay_sys.render_weapons_meter_overlay( new_weapon_level, { 40.f, 80.f }, { 200.f, 20.f } );
      overlay_sys.render_bomb_overlay( bomb_inventory, blast_radius, { 40.f, 120.f } );
      overlay_sys.render_player_candles_overlay( player_candles_count, { 40.f, 160.f } );
      overlay_sys.render_key_count_overlay( player_keys_count, { 40.f, 200.f } );
      overlay_sys.render_relic_count_overlay( player_relic_count, { 40.f, 240.f } );
      if ( m_show_debug_stats )
      {
        overlay_sys.render_player_position_overlay( player_position.position, { 40.f, 300.f } );
        overlay_sys.render_mouse_position_overlay( mouse_world_pos, { 40.f, 340.f } );
        overlay_sys.render_stats_overlay( { 40.f, 360.f }, { 40.f, 380.f }, { 40.f, 400.f } );
      }
      overlay_sys.render_entt_distance_set_overlay( { 40.f, 300.f } );
    }
    // UI Overlays end
  }

  m_window.display();
  // main render end
}

void RenderGameSystem::render_floormap( const sf::Vector2f &offset )
{
  m_floormap.setPosition( offset );
  m_window.draw( m_floormap );
}

void RenderGameSystem::render_player_spawn()
{
  auto spawnarea_view = m_reg->view<Cmp::SpawnAreaSprite, Cmp::Position>();
  for ( auto [entity, spawnarea_cmp, pos_cmp] : spawnarea_view.each() )
  {
    if ( auto it = m_multisprite_map.find( spawnarea_cmp.getType() ); it != m_multisprite_map.end() )
    {
      auto meta_type = it->first;
      auto new_idx = spawnarea_cmp.getTileIndex();
      sf::Vector2f new_scale{ 1.f, 1.f };
      uint8_t new_alpha{ 255 };
      sf::Vector2f new_origin{ 0.f, 0.f };
      float new_angle{ 0.f };
      safe_render_sprite( meta_type, pos_cmp, new_idx, new_scale, new_alpha, new_origin, sf::degrees( new_angle ) );
    }
  }
}

void RenderGameSystem::render_large_obstacles()
{
  auto shrine_view = m_reg->view<Cmp::ShrineSprite, Cmp::Position>();
  for ( auto [entity, shrine_cmp, pos_cmp] : shrine_view.each() )
  {
    if ( auto it = m_multisprite_map.find( shrine_cmp.getType() ); it != m_multisprite_map.end() )
    {
      auto meta_type = it->first;
      auto new_idx = shrine_cmp.getTileIndex();
      auto anim_sprite_cmp = m_reg->try_get<Cmp::SpriteAnimation>( entity );
      if ( anim_sprite_cmp )
      {
        // or use the current frame from the animation component
        new_idx = shrine_cmp.getTileIndex() + anim_sprite_cmp->m_current_frame;
      }

      sf::Vector2f new_scale{ 1.f, 1.f };
      uint8_t new_alpha{ 255 };
      sf::Vector2f new_origin{ 0.f, 0.f };
      float new_angle{ 0.f };
      safe_render_sprite( meta_type, pos_cmp, new_idx, new_scale, new_alpha, new_origin, sf::degrees( new_angle ) );
    }
  }

  auto grave_view = m_reg->view<Cmp::GraveSprite, Cmp::Position>();
  for ( auto [entity, grave_cmp, pos_cmp] : grave_view.each() )
  {
    if ( auto it = m_multisprite_map.find( grave_cmp.getType() ); it != m_multisprite_map.end() )
    {
      auto meta_type = it->first;
      auto new_idx = grave_cmp.getTileIndex();

      sf::Vector2f new_scale{ 1.f, 1.f };
      uint8_t new_alpha{ 255 };
      sf::Vector2f new_origin{ 0.f, 0.f };
      float new_angle{ 0.f };
      safe_render_sprite( meta_type, pos_cmp, new_idx, new_scale, new_alpha, new_origin, sf::degrees( new_angle ) );
    }
  }

  auto large_obstacle_view = m_reg->view<Cmp::LargeObstacle>();
  for ( auto [entity, large_obst_cmp] : large_obstacle_view.each() )
  {
    if ( not large_obst_cmp.are_powers_active() )
    {
      // skip rendering inactive large obstacles
      continue;
    }
    SPDLOG_DEBUG( "Rendering Cmp::LargeObstacle at ({}, {})", large_obst_cmp.position.x, large_obst_cmp.position.y );
    sf::RectangleShape square( sf::Vector2f{ large_obst_cmp.size.x, large_obst_cmp.size.y } );
    square.setFillColor( sf::Color::Transparent );
    square.setOutlineColor( sf::Color::Yellow );
    square.setOutlineThickness( 1.f );
    square.setPosition( { large_obst_cmp.position.x, large_obst_cmp.position.y } );
    m_window.draw( square );
  }
}

void RenderGameSystem::render_npc_containers()
{
  auto npccontainer_view = m_reg->view<Cmp::NpcContainer, Cmp::Position>();
  for ( auto [entity, npccontainer_cmp, pos_cmp] : npccontainer_view.each() )
  {
    if ( auto it = m_multisprite_map.find( npccontainer_cmp.m_type ); it != m_multisprite_map.end() )
    {
      auto meta_type = it->first;
      auto new_idx = npccontainer_cmp.m_tile_index;
      sf::Vector2f new_scale{ 1.f, 1.f };
      uint8_t new_alpha{ 255 };
      sf::Vector2f new_origin{ 0.f, 0.f };
      float new_angle{ 0.f };
      safe_render_sprite( meta_type, pos_cmp, new_idx, new_scale, new_alpha, new_origin, sf::degrees( new_angle ) );
    }
  }
}

void RenderGameSystem::render_loot_containers()
{
  auto lootcontainer_view = m_reg->view<Cmp::LootContainer, Cmp::Position>();
  for ( auto [entity, lootcontainer_cmp, pos_cmp] : lootcontainer_view.each() )
  {
    if ( auto it = m_multisprite_map.find( lootcontainer_cmp.m_type ); it != m_multisprite_map.end() )
    {
      auto meta_type = it->first;
      auto new_idx = lootcontainer_cmp.m_tile_index;
      sf::Vector2f new_scale{ 1.f, 1.f };
      uint8_t new_alpha{ 255 };
      sf::Vector2f new_origin{ 0.f, 0.f };
      float new_angle{ 0.f };
      safe_render_sprite( meta_type, pos_cmp, new_idx, new_scale, new_alpha, new_origin, sf::degrees( new_angle ) );
    }
  }
}

void RenderGameSystem::render_small_obstacles()
{
  // Group similar draw operations to reduce state changes
  std::vector<std::tuple<sf::FloatRect, int, float>> rockPositions;
  std::vector<std::pair<sf::FloatRect, int>> potPositions;
  std::vector<std::pair<sf::FloatRect, int>> bonePositions;
  std::vector<std::pair<sf::FloatRect, int>> npcPositions;
  std::vector<std::pair<sf::FloatRect, int>> disabledPositions;
  std::vector<sf::FloatRect> detonationPositions;

  // Collect all positions first instead of drawing immediately
  auto obst_view = m_reg->view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>( entt::exclude<Cmp::PlayableCharacter> );
  for ( auto [entity, obstacle_cmp, position_cmp, _ob_nb_list] : obst_view.each() )
  {
    // check if obstacle is within the current view (in world coordinates)
    // if ( !is_visible_in_view( m_window.getView(), position_cmp ) ) continue;

    if ( obstacle_cmp.m_enabled )
    {
      if ( obstacle_cmp.m_type == "ROCK" )
      {
        rockPositions.emplace_back( position_cmp, obstacle_cmp.m_tile_index, obstacle_cmp.m_integrity );
      }
      else if ( obstacle_cmp.m_type == "POT" ) { potPositions.emplace_back( position_cmp, obstacle_cmp.m_tile_index ); }
      else if ( obstacle_cmp.m_type == "BONES" ) { bonePositions.emplace_back( position_cmp, obstacle_cmp.m_tile_index ); }
    }

    if ( obstacle_cmp.m_integrity <= 0.0f ) { detonationPositions.push_back( position_cmp ); }
  }

  // Now draw each type in batches
  for ( const auto &[pos_cmp, idx, integrity] : rockPositions )
  {
    auto new_scale = sf::Vector2f{ 1.f, 1.f };
    auto new_alpha = std::lerp( 0.0f, 254.f, integrity );
    safe_render_sprite( "ROCK", pos_cmp, idx, new_scale, new_alpha );
    // sf::RectangleShape player_square( kGridSquareSizePixelsF );
    // player_square.setFillColor( sf::Color::Transparent );
    // player_square.setOutlineColor( sf::Color::Red );
    // player_square.setOutlineThickness( 1.f );
    // player_square.setPosition( pos );
    // m_window.draw( player_square );
  }

  for ( const auto &[pos_cmp, idx] : potPositions )
  {
    safe_render_sprite( "POT", pos_cmp, idx );
  }

  for ( const auto &[pos_cmp, idx] : bonePositions )
  {
    safe_render_sprite( "BONES", pos_cmp, idx );
  }

  // "empty" sprite for detonated objects
  for ( const auto &pos_cmp : detonationPositions )
  {
    safe_render_sprite( "DETONATED", pos_cmp, 0 );
  }

  auto selected_view = m_reg->view<Cmp::SelectedPosition, Cmp::Position>();
  for ( auto [entity, selected_cmp, position_cmp] : selected_view.each() )
  {
    SPDLOG_DEBUG( "Rendering Cmp::SelectedPosition at ({}, {})", selected_cmp.x, selected_cmp.y );
    sf::RectangleShape square( kGridSquareSizePixelsF );
    square.setFillColor( sf::Color::Transparent );
    square.setOutlineColor( sf::Color::White );
    square.setOutlineThickness( 1.f );
    square.setPosition( selected_cmp );
    m_window.draw( square );
  }

  // auto player_start_pos = get_persistent_component<Cmp::Persistent::PlayerStartPosition>();
  // auto player_start_area = Cmp::RectBounds( player_start_pos, kGridSquareSizePixelsF, 5.f,
  //                                           Cmp::RectBounds::ScaleCardinality::BOTH );

  // sf::RectangleShape position_square( player_start_area.size() );
  // position_square.setFillColor( sf::Color::Transparent );
  // position_square.setOutlineColor( sf::Color::Red );
  // position_square.setOutlineThickness( 1.f );
  // position_square.setPosition( player_start_area.position() );
  // m_window.draw( position_square );
}

void RenderGameSystem::render_sinkhole()
{
  std::vector<std::pair<sf::FloatRect, bool>> sinkholePositions;
  auto sinkhole_view = m_reg->view<Cmp::SinkholeCell, Cmp::Position>();
  for ( auto [entity, sinkhole_cmp, position_cmp] : sinkhole_view.each() )
  {
    // if ( !is_visible_in_view( m_window.getView(), position_cmp ) ) continue;
    sinkholePositions.emplace_back( position_cmp, sinkhole_cmp.active );
  }

  for ( const auto &[pos_cmp, active] : sinkholePositions )
  {
    safe_render_sprite( "SINKHOLE", pos_cmp, 0 );
  }
}

void RenderGameSystem::render_corruption()
{
  std::vector<std::pair<sf::FloatRect, bool>> corruptionPositions;
  auto corruption_view = m_reg->view<Cmp::CorruptionCell, Cmp::Position>();
  for ( auto [entity, corruption_cmp, position_cmp] : corruption_view.each() )
  {
    // if ( !is_visible_in_view( m_window.getView(), position_cmp ) ) continue;
    corruptionPositions.emplace_back( position_cmp, corruption_cmp.active );
  }

  for ( const auto &[pos_cmp, active] : corruptionPositions )
  {
    safe_render_sprite( "CORRUPTION", pos_cmp, 0 );
  }
}

void RenderGameSystem::render_wormhole()
{
  auto wormhole_view = m_reg->view<Cmp::Wormhole, Cmp::Position, Cmp::SpriteAnimation>();
  for ( auto [entity, wormhole_cmp, pos_cmp, anim_cmp] : wormhole_view.each() )
  {
    // if ( !is_visible_in_view( m_window.getView(), position_cmp ) ) continue;
    try
    {
      auto &wormhole_sprite = m_multisprite_map.at( "WORMHOLE" );
      // Setup shader
      m_wormhole_shader.update_shader_position( pos_cmp.position + ( kGridSquareSizePixelsF * 0.5f ),
                                                Sprites::ViewFragmentShader::Align::CENTER );

      // Draw background to shader texture
      m_floormap.draw( m_wormhole_shader.get_render_texture(), sf::RenderStates::Default );

      // Draw sprites to shader's render texture using the new function
      auto grid_size = wormhole_sprite.get().get_grid_size();
      for ( float row = 0; row < grid_size.height; ++row )
      {
        for ( float col = 0; col < grid_size.width; ++col )
        {
          sf::Vector2f offset = { ( col - 1 ) * BaseSystem::kGridSquareSizePixels.x,
                                  ( row - 1 ) * BaseSystem::kGridSquareSizePixels.y };
          auto index = anim_cmp.m_current_frame + ( row * grid_size.height + col );
          // auto index = anim_cmp.m_current_frame;

          // dont modify the original pos_cmp, create copy with modified position
          sf::FloatRect offset_pos_cmp{ { pos_cmp.position + offset }, kGridSquareSizePixelsF };
          safe_render_sprite_to_target( m_wormhole_shader.get_render_texture(), "WORMHOLE", offset_pos_cmp, index );
        }
      }

      // Update and draw shader
      Sprites::UniformBuilder builder;
      builder.set( "time", m_wormhole_shader.getElapsedTime().asSeconds() )
          .set( "screenSize", m_wormhole_shader.get_view_size() )
          .set( "centerPosition", m_wormhole_shader.get_view_center() );
      m_wormhole_shader.Sprites::BaseFragmentShader::update( builder );

      m_wormhole_shader.draw( m_window, sf::RenderStates::Default );
    }
    catch ( const std::out_of_range &e )
    {
      SPDLOG_WARN( "Missing wormhole sprite '{}' in map, rendering fallback square", "WORMHOLE" );
      render_fallback_square( pos_cmp, sf::Color::Magenta );
    }

    // // Debug rectangle
    // sf::RectangleShape temp_square( kGridSquareSizePixelsF );
    // temp_square.setPosition( pos_cmp.position );
    // temp_square.setOutlineColor( sf::Color::Red );
    // temp_square.setFillColor( sf::Color::Transparent );
    // temp_square.setOutlineThickness( 1.f );
    // m_window.draw( temp_square );
  }
}

void RenderGameSystem::render_armed()
{
  // render armed obstacles with debug outlines
  auto armed_view = m_reg->view<Cmp::Armed, Cmp::Position>();
  for ( auto [entity, armed_cmp, pos_cmp] : armed_view.each() )
  {
    if ( armed_cmp.m_display_bomb_sprite ) { safe_render_sprite( "BOMB", pos_cmp, 0 ); }

    sf::RectangleShape temp_square( kGridSquareSizePixelsF );
    temp_square.setPosition( pos_cmp.position );
    temp_square.setOutlineColor( sf::Color::Transparent );
    temp_square.setFillColor( sf::Color::Transparent );
    if ( armed_cmp.getElapsedWarningTime() > armed_cmp.m_warning_delay )
    {
      temp_square.setOutlineColor( armed_cmp.m_armed_color );
      temp_square.setFillColor( armed_cmp.m_armed_color );
    }
    temp_square.setOutlineThickness( 1.f );
    m_window.draw( temp_square );

    // debug - F4
    if ( m_show_armed_obstacles )
    {
      sf::Text text( m_font, "", 12 );
      text.setString( std::to_string( armed_cmp.m_index ) );
      text.setPosition( pos_cmp.position );
      m_window.draw( text );
    }
  }
}

void RenderGameSystem::render_loot()
{
  auto loot_view = m_reg->view<Cmp::Loot, Cmp::Position>();
  for ( auto [entity, loot_cmp, pos_cmp] : loot_view.each() )
  {
    // clang-format off
    if ( loot_cmp.m_type == "EXTRA_HEALTH" ) { RenderSystem::safe_render_sprite( "EXTRA_HEALTH", pos_cmp, loot_cmp.m_tile_index ); }
    else if ( loot_cmp.m_type == "EXTRA_BOMBS" ) { safe_render_sprite( "EXTRA_BOMBS", pos_cmp, loot_cmp.m_tile_index ); }
    else if ( loot_cmp.m_type == "INFINI_BOMBS" ) { safe_render_sprite( "INFINI_BOMBS", pos_cmp, loot_cmp.m_tile_index ); }
    else if ( loot_cmp.m_type == "CHAIN_BOMBS" ) { safe_render_sprite( "CHAIN_BOMBS", pos_cmp, loot_cmp.m_tile_index ); }
    else if ( loot_cmp.m_type == "LOWER_WATER" ) { safe_render_sprite( "LOWER_WATER", pos_cmp, loot_cmp.m_tile_index ); }
    else if ( loot_cmp.m_type == "WEAPON_BOOST" ) { safe_render_sprite( "WEAPON_BOOST", pos_cmp, loot_cmp.m_tile_index ); }
    else if ( loot_cmp.m_type == "CANDLE_DROP" ) { safe_render_sprite( "CANDLE_DROP", pos_cmp, loot_cmp.m_tile_index ); }
    else if ( loot_cmp.m_type == "KEY_DROP" ) { safe_render_sprite( "KEY_DROP", pos_cmp, loot_cmp.m_tile_index ); }
    else if ( loot_cmp.m_type == "RELIC_DROP" ) { safe_render_sprite( "RELIC_DROP", pos_cmp, loot_cmp.m_tile_index ); }
    else { SPDLOG_WARN( "Unknown loot type: {}", loot_cmp.m_type ); }
    // clang-format on
  }
}

void RenderGameSystem::render_walls()
{
  // draw walls and door frames
  auto wall_view = m_reg->view<Cmp::Wall, Cmp::Position>();
  for ( auto [entity, wall_cmp, pos_cmp] : wall_view.each() )
  {
    // if ( !is_visible_in_view( m_window.getView(), position_cmp ) ) continue;
    sf::Vector2f new_scale{ 1.f, 1.f };
    uint8_t new_alpha{ 255 };
    sf::Vector2f new_origin{ 0.f, 0.f };
    float angle{ 0.f };

    safe_render_sprite( "WALL", pos_cmp, wall_cmp.m_tile_index, new_scale, new_alpha, new_origin, sf::degrees( angle ) );
  }

  // draw entrance
  auto entrance_door_view = m_reg->view<Cmp::Door, Cmp::Position>( entt::exclude<Cmp::Exit> );
  for ( auto [entity, door_cmp, pos_cmp] : entrance_door_view.each() )
  {
    // if ( !is_visible_in_view( m_window.getView(), position_cmp ) ) continue;
    sf::Vector2f new_scale{ 1.f, 1.f };
    uint8_t new_alpha{ 255 };
    sf::Vector2f new_origin{ 0.f, 0.f };
    float angle{ 0.f };

    safe_render_sprite( "WALL", pos_cmp, door_cmp.m_tile_index, new_scale, new_alpha, new_origin, sf::degrees( angle ) );
  }

  auto exit_door_view = m_reg->view<Cmp::Door, Cmp::Position, Cmp::Exit>();
  for ( auto [entity, door_cmp, pos_cmp, exit_cmp] : exit_door_view.each() )
  {
    // if ( !is_visible_in_view( m_window.getView(), position_cmp ) ) continue;
    auto half_width_px = BaseSystem::kGridSquareSizePixels.x / 2.f;
    auto half_height_px = BaseSystem::kGridSquareSizePixels.y / 2.f;

    // dont modify the original pos_cmp, create copy with modified position
    sf::FloatRect new_pos{ pos_cmp.position + sf::Vector2f{ half_width_px, half_height_px }, kGridSquareSizePixelsF };

    sf::Vector2f new_scale{ 1.f, 1.f };
    uint8_t new_alpha{ 255 };
    sf::Vector2f new_origin{ half_width_px, half_height_px };
    float angle{ 0.f };

    safe_render_sprite( "WALL", new_pos, door_cmp.m_tile_index, new_scale, new_alpha, new_origin, sf::degrees( angle ) );
  }
}

void RenderGameSystem::render_player()
{
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::PCDetectionBounds,
                                 Cmp::SpriteAnimation>();
  for ( auto [entity, pc_cmp, pc_pos_cmp, dir_cmp, pc_detection_bounds, anim_cmp] : player_view.each() )
  {
    int sprite_index;

    if ( dir_cmp == sf::Vector2f( 0.0f, 0.0f ) )
    {
      // Use static frame when not moving
      sprite_index = anim_cmp.m_base_frame;
    }
    else
    {
      // Use animated frame: base_frame + current_frame
      sprite_index = anim_cmp.m_base_frame + anim_cmp.m_current_frame;
    }
    // dont modify the original pos_cmp, create copy with modified position
    sf::FloatRect new_pos{ { pc_pos_cmp.position.x + dir_cmp.x_offset, pc_pos_cmp.position.y }, kGridSquareSizePixelsF };

    auto &pc_damage_cooldown = get_persistent_component<Cmp::Persistent::PcDamageDelay>();
    bool is_in_damage_cooldown = pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value();

    // Only render if not in cooldown OR if in cooldown and blink is visible
    if ( !is_in_damage_cooldown ||
         ( is_in_damage_cooldown &&
           static_cast<int>( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asMilliseconds() / 100 ) % 2 == 0 ) )
    {
      safe_render_sprite( "PLAYER", new_pos, sprite_index );
    }

    if ( m_show_path_distances )
    {
      sf::RectangleShape pc_square( pc_detection_bounds.size() );
      pc_square.setFillColor( sf::Color::Transparent );
      pc_square.setOutlineColor( sf::Color::Green );
      pc_square.setOutlineThickness( 1.f );
      pc_square.setPosition( pc_detection_bounds.position() );
      m_window.draw( pc_square );
    }

    // auto half_sprite_size = kGridSquareSizePixelsF;
    // auto player_horizontal_bounds = Cmp::RectBounds( pc_pos_cmp, half_sprite_size, 1.5f,
    //                                                  Cmp::RectBounds::ScaleCardinality::HORIZONTAL );
    // auto player_vertical_bounds = Cmp::RectBounds( pc_pos_cmp, half_sprite_size, 1.5f,
    //                                                Cmp::RectBounds::ScaleCardinality::VERTICAL );

    // // auto player_hitbox = Cmp::RectBounds( pc_pos_cmp, sf::Vector2f{ BaseSystem::kGridSquareSizePixels
    // },
    // //                                       1.2f );
    // // Debug: Draw a green rectangle around the player position
    // sf::RectangleShape player_square( player_horizontal_bounds.size() );
    // player_square.setFillColor( sf::Color::Transparent );
    // player_square.setOutlineColor( sf::Color::Green );
    // player_square.setOutlineThickness( 1.f );
    // player_square.setPosition( player_horizontal_bounds.position() );
    // m_window.draw( player_square );

    // sf::RectangleShape player_square2( player_vertical_bounds.size() );
    // player_square2.setFillColor( sf::Color::Transparent );
    // player_square2.setOutlineColor( sf::Color::Blue );
    // player_square2.setOutlineThickness( 1.f );
    // player_square2.setPosition( player_vertical_bounds.position() );
    // m_window.draw( player_square2 );
  }
}

void RenderGameSystem::render_player_footsteps()
{

  // render all footsteps
  auto footstep_view = m_reg->view<Cmp::FootStepTimer, Cmp::FootStepAlpha, Cmp::Position, Cmp::Direction>();
  for ( auto [entity, timer, alpha, pos_cmp, direction] : footstep_view.each() )
  {
    std::size_t new_idx = 0;
    uint8_t new_alpha{ alpha.m_alpha };
    sf::Vector2f new_scale{ 1.f, 1.f };
    // we're changing the origin to be the center of the sprite so that
    // rotation happens around the center, this means we also need to
    // offset the position to make it look convincing depending on direction of movement
    sf::Vector2f new_origin{ BaseSystem::kGridSquareSizePixels.x / 2.f, BaseSystem::kGridSquareSizePixels.y / 2.f };
    sf::FloatRect new_pos{ pos_cmp.position, kGridSquareSizePixelsF };
    // moving in right direction: place footsteps to bottom-left of player position
    if ( direction == sf::Vector2f( 1.f, 0.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.25f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.75f ) };
    }
    // moving in left direction: place footsteps to bottom-right of player position
    else if ( direction == sf::Vector2f( -1.f, 0.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.75f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.75f ) };
    }
    // moving diagonally: down/left
    else if ( direction == sf::Vector2f( -1.f, 1.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.75f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.75f ) };
    }
    // moving diagonally: down/right
    else if ( direction == sf::Vector2f( 1.f, 1.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.5f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.8f ) };
    }
    // moving diagonally: up/left
    else if ( direction == sf::Vector2f( -1.f, -1.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.5f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.8f ) };
    }
    // moving diagonally: up/right
    else if ( direction == sf::Vector2f( 1.f, -1.f ) )
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.5f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.8f ) };
    }
    // moving in up/down direction: place footsteps to center of player position
    else
    {
      new_pos.position = { pos_cmp.position.x + ( BaseSystem::kGridSquareSizePixels.x * 0.5f ),
                           pos_cmp.position.y + ( BaseSystem::kGridSquareSizePixels.y * 0.5f ) };
    }
    // only set rotation if direction is not zero vector
    sf::Angle new_angle;
    if ( direction != sf::Vector2f( 0.f, 0.f ) ) { new_angle = direction.angle(); }

    safe_render_sprite( "FOOTSTEPS", new_pos, new_idx, new_scale, new_alpha, new_origin, new_angle );
  }
}

void RenderGameSystem::render_npc()
{
  for ( auto [entity, npc_cmp, pos_cmp, npc_sb_cmp, dir_cmp, anim_cmp] :
        m_reg->view<Cmp::NPC, Cmp::Position, Cmp::NPCScanBounds, Cmp::Direction, Cmp::SpriteAnimation>().each() )
  {
    // if ( !is_visible_in_view( m_window.getView(), position_cmp ) ) continue;
    // flip and x-axis offset the sprite depending on the direction
    if ( dir_cmp.x > 0 )
    {
      dir_cmp.x_scale = 1.f;
      dir_cmp.x_offset = 0.f;
    }
    else if ( dir_cmp.x < 0 )
    {
      dir_cmp.x_scale = -1.f;
      dir_cmp.x_offset = BaseSystem::kGridSquareSizePixels.x;
    }
    else
    {
      dir_cmp.x_scale = dir_cmp.x_scale; // keep last known direction
      dir_cmp.x_offset = dir_cmp.x_offset;
    }

    sf::Vector2f new_scale{ dir_cmp.x_scale, 1.f };
    sf::FloatRect new_position{ sf::Vector2f{ pos_cmp.position.x + dir_cmp.x_offset, pos_cmp.position.y }, kGridSquareSizePixelsF };
    // get the correct sprite index based on animation frame
    safe_render_sprite( npc_cmp.m_type, new_position, anim_cmp.m_current_frame, new_scale );

    // show npc scan distance
    if ( m_show_path_distances )
    {
      sf::RectangleShape npc_square( npc_sb_cmp.size() );
      npc_square.setFillColor( sf::Color::Transparent );
      npc_square.setOutlineColor( sf::Color::Red );
      npc_square.setOutlineThickness( 1.f );
      npc_square.setPosition( npc_sb_cmp.position() );
      m_window.draw( npc_square );
    }

    // sf::RectangleShape player_square( kGridSquareSizePixelsF );
    // player_square.setFillColor( sf::Color::Transparent );
    // player_square.setOutlineColor( sf::Color::Red );
    // player_square.setOutlineThickness( 1.f );
    // player_square.setPosition( pos );
    // m_window.draw( player_square );
  }
}

void RenderGameSystem::render_explosions()
{
  auto explosion_view = m_reg->view<Cmp::NpcDeathPosition, Cmp::SpriteAnimation>();
  for ( auto [entity, pos_cmp, anim_cmp] : explosion_view.each() )
  {
    // Always render the current frame
    sf::FloatRect npc_death_pos{ pos_cmp, kGridSquareSizePixelsF };
    safe_render_sprite( "EXPLOSION", npc_death_pos, anim_cmp.m_current_frame );
  }
}

void RenderGameSystem::render_flood_waters( sf::FloatRect player_position )
{

  m_water_shader.update( { player_position.position.x - m_water_shader.get_texture_size().x / 2.f,
                           player_position.position.y - m_water_shader.get_texture_size().y / 2.f } );
  m_window.draw( m_water_shader );
}

void RenderGameSystem::render_mist( sf::FloatRect player_position )
{
  m_mist_shader.update( { player_position.position.x - m_mist_shader.get_texture_size().x / 2.f,
                          player_position.position.y - m_mist_shader.get_texture_size().y / 2.f },
                        0.25 ); // Set the alpha value

  m_pulsing_shader.update( { player_position.position.x - m_pulsing_shader.get_texture_size().x / 2.f,
                             player_position.position.y - m_pulsing_shader.get_texture_size().y / 2.f },
                           0.5f ); // Set the alpha value

  m_window.draw( m_mist_shader );
  m_window.draw( m_pulsing_shader );
}

void RenderGameSystem::render_player_distances_on_npc()
{
  if ( !m_show_path_distances ) return;

  // for (auto [entt, npc_position] : m_reg->view<Cmp::Position>().each())
  // {
  //     // sf::Text distance_text(m_font, "", 10);
  //     // if( player_distance_to_npc.distance ==
  //     std::numeric_limits<unsigned int>::max() ) {
  //     //     continue;
  //     // } else {
  //     //
  //     distance_text.setString(std::to_string(player_distance_to_npc.distance));
  //     // }

  //     // distance_text.setPosition(npc_position + sf::Vector2f{5.f, 0.f});
  //     // distance_text.setFillColor(sf::Color::White);
  //     // distance_text.setOutlineColor(sf::Color::Black);
  //     // distance_text.setOutlineThickness(2.f);
  //     // m_window.draw(distance_text);

  // }
}

void RenderGameSystem::render_player_distances_on_obstacles()
{
  if ( !m_show_path_distances ) return;
  auto obstacle_view = m_reg->view<Cmp::Position, Cmp::PlayerDistance>();
  for ( auto [ob_entt, pos_cmp, player_dist_cmp] : obstacle_view.each() )
  {
    sf::Text distance_text( m_font, "", 10 );
    distance_text.setString( std::to_string( player_dist_cmp.distance ) );
    distance_text.setPosition( pos_cmp.position + sf::Vector2f{ 5.f, 0.f } );
    distance_text.setFillColor( sf::Color::White );
    distance_text.setOutlineColor( sf::Color::Black );
    distance_text.setOutlineThickness( 2.f );
    m_window.draw( distance_text );
  }
}

void RenderGameSystem::render_npc_distances_on_obstacles()
{
  if ( !m_show_path_distances ) return;

  auto entt_distance_map_view = m_reg->view<Cmp::EnttDistanceMap>();

  for ( auto [npc_entt, distance_map] : entt_distance_map_view.each() )
  {
    for ( auto [obstacle_entt, distance] : distance_map )
    {
      auto obstacle_position = m_reg->try_get<Cmp::Position>( obstacle_entt );
      if ( not obstacle_position ) continue;

      sf::Text distance_text( m_font, "", 10 );

      distance_text.setString( "+" );

      distance_text.setPosition( ( *obstacle_position ).position );
      distance_text.setFillColor( sf::Color::White );
      distance_text.setOutlineColor( sf::Color::Black );
      distance_text.setOutlineThickness( 2.f );
      m_window.draw( distance_text );
    }
  }
}

void RenderGameSystem::render_positions()
{
  auto position_view = m_reg->view<Cmp::Position>();
  for ( auto [entity, pos_cmp] : position_view.each() )
  {
    sf::RectangleShape position_marker( kGridSquareSizePixelsF );
    position_marker.setPosition( pos_cmp.position );
    position_marker.setFillColor( sf::Color::Transparent );
    position_marker.setOutlineColor( sf::Color::Green );
    position_marker.setOutlineThickness( 1.f );
    m_window.draw( position_marker );
  }
}

void RenderGameSystem::render_arrow_compass()
{
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
  auto exit_view = m_reg->view<Cmp::Exit, Cmp::Position>();

  for ( auto [player_entity, pc_cmp, pc_pos_cmp] : player_view.each() )
  {
    for ( auto [exit_entity, exit_cmp, exit_pos_cmp] : exit_view.each() )
    {
      // only show the compass arrow if we unlocked the door (to reward player and help them find exit)
      if ( exit_cmp.m_locked ) continue;

      // dont show the compass arrow pointing to the exit if the exit is on-screen....we can see it
      if ( is_visible_in_view( getGameView(), exit_pos_cmp ) ) return;

      auto player_pos_center = pc_pos_cmp.getCenter();
      sf::Vector2f exit_pos_center = exit_pos_cmp.getCenter();
      sf::Vector2f direction = ( exit_pos_center - player_pos_center ).normalized();

      // Get view bounds in world coordinates
      sf::Vector2f view_center = m_local_view.getCenter();
      sf::Vector2f view_size = m_local_view.getSize();
      sf::FloatRect view_bounds{ { view_center.x - view_size.x / 2.0f, view_center.y - view_size.y / 2.0f }, view_size };

      // Add margin from edge
      float margin = 32.0f;
      view_bounds.position.x += margin;
      view_bounds.position.y += margin;
      view_bounds.size.x -= margin * 2.0f;
      view_bounds.size.y -= margin * 2.0f;

      // Calculate intersection with screen bounds
      sf::Vector2f arrow_position = player_pos_center;

      // Calculate distances to each edge
      float t_left = ( view_bounds.position.x - player_pos_center.x ) / direction.x;
      float t_right = ( view_bounds.position.x + view_bounds.size.x - player_pos_center.x ) / direction.x;
      float t_top = ( view_bounds.position.y - player_pos_center.y ) / direction.y;
      float t_bottom = ( view_bounds.position.y + view_bounds.size.y - player_pos_center.y ) / direction.y;

      // Find the smallest positive t (closest intersection)
      float t = std::numeric_limits<float>::max();
      if ( t_left > 0 ) t = std::min( t, t_left );
      if ( t_right > 0 ) t = std::min( t, t_right );
      if ( t_top > 0 ) t = std::min( t, t_top );
      if ( t_bottom > 0 ) t = std::min( t, t_bottom );

      // Calculate final arrow position at screen edge
      if ( t < std::numeric_limits<float>::max() ) { arrow_position = player_pos_center + direction * t; }

      // Use SFML's angle() function to get the angle directly
      auto angle_radians = direction.angle();

      // Center the arrow sprite at the calculated position
      sf::FloatRect arrow_rect{ arrow_position - sf::Vector2f{ kGridSquareSizePixelsF.x / 2.0f, kGridSquareSizePixelsF.y / 2.0f },
                                kGridSquareSizePixelsF };

      // Map sin(time) from [-1, 1] to [0.2, 1.0]
      // Formula: min + (max - min) * (sin(freq * time) + 1) / 2
      auto time = m_compass_osc_clock.getElapsedTime().asSeconds();
      auto sine = std::sin( m_compass_freq * time );
      float oscillating_scale = m_compass_min_scale + ( m_compass_max_scale - m_compass_min_scale ) * ( sine + 1.0f ) / 2.0f;
      auto scale = sf::Vector2f{ oscillating_scale, oscillating_scale };

      auto sprite_index = 0;
      auto alpha = 255;
      auto origin = sf::Vector2f{ kGridSquareSizePixelsF.x / 2.0f, kGridSquareSizePixelsF.y / 2.0f };

      safe_render_sprite( "ARROW", arrow_rect, sprite_index, scale, alpha, origin, angle_radians );
    }
  }
}

void RenderGameSystem::update_view_center( sf::View &view, [[maybe_unused]] const Cmp::Position &player_pos,
                                           [[maybe_unused]] float smoothFactor )
{
  // const float kHalfViewWidth = view.getSize().x * 0.5f;
  // const float kHalfViewHeight = view.getSize().y * 0.5f;

  // // Calculate the maximum allowed camera positions
  // float maxX = kDisplaySize.x - kHalfViewWidth;
  // float maxY = kDisplaySize.y - kHalfViewHeight;

  // // Calculate new camera position
  // float newX = std::clamp( player_pos.x, kHalfViewWidth, maxX );
  // float newY = std::clamp( player_pos.y, kHalfViewHeight, maxY );

  // // Smoothly interpolate to the new position
  // sf::Vector2f currentCenter = view.getCenter();

  // view.setCenter( { currentCenter.x + ( newX - currentCenter.x ) * smoothFactor,
  //                   currentCenter.y + ( newY - currentCenter.y ) * smoothFactor } );
  view.setCenter( player_pos.position );
}

} // namespace ProceduralMaze::Sys