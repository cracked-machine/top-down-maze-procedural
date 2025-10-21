#include <Door.hpp>
#include <Exit.hpp>
#include <FootStepAlpha.hpp>
#include <FootStepTimer.hpp>
#include <HazardFieldCell.hpp>

#include <LargeObstacle.hpp>
#include <MultiSprite.hpp>
#include <NpcDeathPosition.hpp>
#include <Persistent/NpcDeathAnimFramerate.hpp>
#include <Persistent/PlayerStartPosition.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <RenderSystem.hpp>
#include <ReservedPosition.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <SinkholeCell.hpp>
#include <SpriteAnimation.hpp>
#include <Systems/RenderGameSystem.hpp>
#include <Wall.hpp>
#include <Wormhole.hpp>
#include <string>

namespace ProceduralMaze::Sys {

RenderGameSystem::RenderGameSystem( ProceduralMaze::SharedEnttRegistry reg )
    : RenderSystem( reg )
{
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
  update_view_center( m_local_view, start_pos, kStartGameSmoothFactor );
  update_view_center( m_minimap_view, start_pos, kStartGameSmoothFactor );
}

void RenderGameSystem::init_multisprites()
{
  using namespace Sprites;
  auto &factory = get_persistent_component<std::shared_ptr<SpriteFactory>>();
  for ( auto type : factory->get_all_sprite_types() )
  {
    m_multisprite_map[type] = factory->get_multisprite_by_type( type );
    if ( !m_multisprite_map[type] )
    {
      SPDLOG_CRITICAL( "Unable to get {} from SpriteFactory", type );
      std::terminate();
    }
  }
}

void RenderGameSystem::render_game( [[maybe_unused]] sf::Time deltaTime )
{
  using namespace Sprites;

  // check for updates to the System modes
  for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
  {
    m_show_path_distances = _sys.show_path_distances;
    m_show_armed_obstacles = _sys.show_armed_obstacles;
  }

  // main render begin
  getWindow().clear();
  {
    // local view begin - this shows only a `LOCAL_MAP_VIEW_SIZE` of the game
    // world
    getWindow().setView( m_local_view );
    {
      sf::Vector2f player_position{ 0.f, 0.f };

      // move the local view position to equal the player position

      for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
      {
        if ( _sys.player_stuck )
        {
          // reset the center if player is stuck
          m_local_view.setCenter( { kLocalMapViewSize.x * 0.5f, kDisplaySize.y * 0.5f } );
          _sys.player_stuck = false;
        }
        else
        {
          for ( auto [entity, _pc, _pos] : m_reg->view<Cmp::PlayableCharacter, Cmp::Position>().each() )
          {
            player_position = _pos;
            update_view_center( m_local_view, _pos );
          }
        }
      }

      // draw the background
      render_floormap( { 0, 0 } );

      // // now post-process the floormap with the ViewFragmentShader
      // m_sand_storm_shader.update_shader_view_and_position(
      //     player_position + ( sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } * 0.5f ),
      //     ViewFragmentShader::Align::CENTER );
      // m_floormap.draw( m_sand_storm_shader.get_render_texture(), sf::RenderStates::Default );

      // UniformBuilder builder;
      // builder.set( "time", m_sand_storm_shader.getElapsedTime().asSeconds() )
      //     .set( "screenSize", m_sand_storm_shader.get_view_size() )
      //     .set( "centerPosition", m_sand_storm_shader.get_view_center() );
      // m_sand_storm_shader.Sprites::BaseFragmentShader::update( builder );

      // getWindow().draw( m_sand_storm_shader );

      // now draw everything else on top of the sand shader
      render_small_obstacles();

      render_sinkhole();
      render_corruption();
      render_wormhole();
      render_armed();
      render_loot();
      render_walls();
      render_player_footsteps();
      render_player();
      render_npc();
      render_large_obstacles();
      render_explosions();
      render_flood_waters();
      render_player_distances_on_npc();
      render_player_distances_on_obstacles();
    }
    // local view end

    // minimap view begin - this show a quarter of the game world but in a
    // much smaller scale
    getWindow().setView( m_minimap_view );
    {
      render_floormap( { 0, kMapGridOffset.y * Sprites::MultiSprite::kDefaultSpriteDimensions.y } );
      render_small_obstacles();

      render_sinkhole();
      render_corruption();
      render_wormhole();
      render_armed();
      render_loot();
      render_walls();
      render_player();
      render_npc();
      render_large_obstacles();
      render_flood_waters();

      // update the minimap view center based on player position
      // reset the center if player is stuck
      for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
      {
        if ( _sys.player_stuck )
        {
          m_minimap_view.setCenter( { kDisplaySize.x * 0.5f, kDisplaySize.y * 0.5f } );
          _sys.player_stuck = false;
        }
        else
        {
          for ( auto [entity, _pc, _pos] : m_reg->view<Cmp::PlayableCharacter, Cmp::Position>().each() )
          {
            update_view_center( m_minimap_view, _pos );
          }
        }
      }
    }
    // minimap view end

    // UI Overlays begin (these will always be displayed no matter where the
    // player moves)
    getWindow().setView( getWindow().getDefaultView() );
    {
      auto minimap_border = sf::RectangleShape( {
          getWindow().getSize().x * kMiniMapViewZoomFactor, // 25% of screen width
          getWindow().getSize().y * kMiniMapViewZoomFactor  // 25% of screen height
      } );
      minimap_border.setPosition( { getWindow().getSize().x - minimap_border.getSize().x, 0.f } );

      minimap_border.setFillColor( sf::Color::Transparent );
      minimap_border.setOutlineColor( sf::Color::White );
      minimap_border.setOutlineThickness( 2.f );
      getWindow().draw( minimap_border );

      for ( auto [_entt, _pc] : m_reg->view<Cmp::PlayableCharacter>().each() )
      {
        m_overlay_sys.render_health_overlay( _pc.health, { 40.f, 20.f }, { 200.f, 20.f } );
        m_overlay_sys.render_bomb_overlay( _pc.bomb_inventory, { 40.f, 120.f } );
        m_overlay_sys.render_bomb_radius_overlay( _pc.blast_radius, { 40.f, 150.f } );
      }

      for ( auto [_entt, water_level] : m_reg->view<Cmp::WaterLevel>().each() )
      {
        m_overlay_sys.render_water_level_meter_overlay( water_level.m_level, { 40.f, 70.f }, { 200.f, 20.f } );
      }

      auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
      for ( auto [entity, pc_cmp, pos_cmp] : player_view.each() )
      {
        m_overlay_sys.render_player_position_overlay( pos_cmp, { 40.f, 220.f } );
      }

      m_overlay_sys.render_entt_distance_set_overlay( { 40.f, 300.f } );
    }
    // UI Overlays end
  }

  getWindow().display();
  // main render end
}

void RenderGameSystem::render_floormap( const sf::Vector2f &offset )
{
  m_floormap.setPosition( offset );
  getWindow().draw( m_floormap );
}

void RenderGameSystem::render_large_obstacles()
{
  auto reserved_view = m_reg->view<Cmp::ReservedPosition>();
  for ( auto [entity, reserved_cmp] : reserved_view.each() )
  {
    // it->first: SpriteMetaType (aka std::string)
    // it->second: optional<MultiSprite>
    if ( auto it = m_multisprite_map.find( reserved_cmp.m_type );
         it != m_multisprite_map.end() && it->second.has_value() )
    {
      auto meta_type = it->first;
      // when not activated use the default sprite index
      auto new_idx = reserved_cmp.m_sprite_index;
      auto anim_sprite_cmp = m_reg->try_get<Cmp::SpriteAnimation>( entity );
      if ( anim_sprite_cmp )
      {
        // or use the current frame from the animation component
        new_idx = reserved_cmp.m_sprite_index + anim_sprite_cmp->m_current_frame;
        SPDLOG_TRACE( "Rendering animated ReservedPosition for entity {} at index {}, frame {}",
                      static_cast<int>( entity ), reserved_cmp.m_sprite_index, anim_sprite_cmp->m_current_frame );
      }
      // if ( reserved_cmp.is_animated() )
      //   new_idx = reserved_cmp.m_sprite_index + it->second.value().get_sprites_per_frame();

      sf::Vector2f new_scale{ 1.f, 1.f };
      uint8_t new_alpha{ 255 };
      sf::Vector2f new_origin{ 0.f, 0.f };
      float new_angle{ 0.f };
      safe_render_sprite( meta_type, reserved_cmp, new_idx, new_scale, new_alpha, new_origin,
                          sf::degrees( new_angle ) );
    }

    // if ( reserved_cmp.is_animated() )
    // {
    //   SPDLOG_TRACE( "Rendering Cmp::ReservedPosition at ({}, {})", reserved_cmp.x, reserved_cmp.y );
    //   sf::RectangleShape square( sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } );
    //   square.setFillColor( sf::Color::Transparent );
    //   square.setOutlineColor( sf::Color::Blue );
    //   square.setOutlineThickness( 1.f );
    //   square.setPosition( reserved_cmp );
    //   getWindow().draw( square );
    // }
  }

  // auto large_obstacle_view = m_reg->view<Cmp::LargeObstacle>();
  // for ( auto [entity, large_obst_cmp] : large_obstacle_view.each() )
  // {
  //   if ( not large_obst_cmp.m_powers_active || large_obst_cmp.m_powers_extinct )
  //   {
  //     // skip rendering inactive large obstacles
  //     continue;
  //   }
  //   SPDLOG_DEBUG( "Rendering Cmp::LargeObstacle at ({}, {})", large_obst_cmp.position.x, large_obst_cmp.position.y );
  //   sf::RectangleShape square( sf::Vector2f{ large_obst_cmp.size.x, large_obst_cmp.size.y } );
  //   square.setFillColor( sf::Color::Transparent );
  //   square.setOutlineColor( sf::Color::Green );
  //   square.setOutlineThickness( 2.f );
  //   square.setPosition( { large_obst_cmp.position.x, large_obst_cmp.position.y } );
  //   getWindow().draw( square );
  // }
}

void RenderGameSystem::render_small_obstacles()
{

  // Group similar draw operations to reduce state changes
  std::vector<std::pair<sf::Vector2f, int>> rockPositions;
  std::vector<std::pair<sf::Vector2f, int>> potPositions;
  std::vector<std::pair<sf::Vector2f, int>> bonePositions;
  std::vector<std::pair<sf::Vector2f, int>> npcPositions;
  std::vector<std::pair<sf::Vector2f, int>> disabledPositions;
  std::vector<sf::Vector2f> detonationPositions;

  // Collect all positions first instead of drawing immediately
  auto obst_view = m_reg->view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>(
      entt::exclude<Cmp::PlayableCharacter, Cmp::ReservedPosition> );
  for ( auto [entity, obstacle_cmp, position_cmp, _ob_nb_list] : obst_view.each() )
  {
    if ( obstacle_cmp.m_enabled )
    {
      if ( obstacle_cmp.m_type == "ROCK" ) { rockPositions.emplace_back( position_cmp, obstacle_cmp.m_tile_index ); }
      else if ( obstacle_cmp.m_type == "POT" ) { potPositions.emplace_back( position_cmp, obstacle_cmp.m_tile_index ); }
      else if ( obstacle_cmp.m_type == "BONES" )
      {
        bonePositions.emplace_back( position_cmp, obstacle_cmp.m_tile_index );
      }
    }

    if ( obstacle_cmp.m_broken ) { detonationPositions.push_back( position_cmp ); }
  }

  // Now draw each type in batches
  for ( const auto &[pos, idx] : rockPositions )
  {
    safe_render_sprite( "ROCK", pos, idx );
    // sf::RectangleShape player_square( sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } );
    // player_square.setFillColor( sf::Color::Transparent );
    // player_square.setOutlineColor( sf::Color::Red );
    // player_square.setOutlineThickness( 1.f );
    // player_square.setPosition( pos );
    // getWindow().draw( player_square );
  }

  for ( const auto &[pos, idx] : potPositions )
  {
    safe_render_sprite( "POT", pos, idx );
  }

  for ( const auto &[pos, idx] : bonePositions )
  {
    safe_render_sprite( "BONES", pos, idx );
  }

  // "empty" sprite for detonated objects
  for ( const auto &pos : detonationPositions )
  {
    safe_render_sprite( "DETONATED", pos, 0 );
  }
}

void RenderGameSystem::render_sinkhole()
{
  std::vector<std::pair<sf::Vector2f, bool>> sinkholePositions;
  auto sinkhole_view = m_reg->view<Cmp::SinkholeCell, Cmp::Position>();
  for ( auto [entity, sinkhole_cmp, position_cmp] : sinkhole_view.each() )
  {
    sinkholePositions.emplace_back( position_cmp, sinkhole_cmp.active );
  }

  for ( const auto &[pos, active] : sinkholePositions )
  {
    safe_render_sprite( "SINKHOLE", pos, 0 );
  }
}

void RenderGameSystem::render_corruption()
{
  std::vector<std::pair<sf::Vector2f, bool>> corruptionPositions;
  auto corruption_view = m_reg->view<Cmp::CorruptionCell, Cmp::Position>();
  for ( auto [entity, corruption_cmp, position_cmp] : corruption_view.each() )
  {
    corruptionPositions.emplace_back( position_cmp, corruption_cmp.active );
  }

  for ( const auto &[pos, active] : corruptionPositions )
  {
    safe_render_sprite( "CORRUPTION", pos, 0 );
  }
}

void RenderGameSystem::render_wormhole()
{
  auto wormhole_view = m_reg->view<Cmp::Wormhole, Cmp::Position, Cmp::SpriteAnimation>();
  for ( auto [entity, wormhole_cmp, position_cmp, anim_cmp] : wormhole_view.each() )
  {
    try
    {
      auto &wormhole_sprite = m_multisprite_map.at( "WORMHOLE" );
      if ( !wormhole_sprite.has_value() )
      {
        SPDLOG_WARN( "Wormhole sprite exists in map but has no value" );
        render_fallback_square( position_cmp, sf::Color::Yellow );
        continue;
      }

      // Setup shader
      m_wormhole_shader.update_shader_position(
          position_cmp + ( sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } * 0.5f ),
          Sprites::ViewFragmentShader::Align::CENTER );

      // Draw background to shader texture
      m_floormap.draw( m_wormhole_shader.get_render_texture(), sf::RenderStates::Default );

      // Draw sprites to shader's render texture using the new function
      auto grid_size = wormhole_sprite->get_grid_size();
      for ( float row = 0; row < grid_size.height; ++row )
      {
        for ( float col = 0; col < grid_size.width; ++col )
        {
          sf::Vector2f offset = { ( col - 1 ) * Sprites::MultiSprite::kDefaultSpriteDimensions.x,
                                  ( row - 1 ) * Sprites::MultiSprite::kDefaultSpriteDimensions.y };
          auto index = anim_cmp.m_current_frame + ( row * grid_size.height + col );

          // Much cleaner: render to shader's render texture
          safe_render_sprite_to_target( m_wormhole_shader.get_render_texture(), "WORMHOLE", position_cmp + offset,
                                        index );
        }
      }

      // Update and draw shader
      Sprites::UniformBuilder builder;
      builder.set( "time", m_wormhole_shader.getElapsedTime().asSeconds() )
          .set( "screenSize", m_wormhole_shader.get_view_size() )
          .set( "centerPosition", m_wormhole_shader.get_view_center() );
      m_wormhole_shader.Sprites::BaseFragmentShader::update( builder );

      m_wormhole_shader.draw( getWindow(), sf::RenderStates::Default );
    }
    catch ( const std::out_of_range &e )
    {
      SPDLOG_WARN( "Missing wormhole sprite '{}' in map, rendering fallback square", "WORMHOLE" );
      render_fallback_square( position_cmp, sf::Color::Magenta );
    }

    // // Debug rectangle
    // sf::RectangleShape temp_square( sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } );
    // temp_square.setPosition( position_cmp );
    // temp_square.setOutlineColor( sf::Color::Red );
    // temp_square.setFillColor( sf::Color::Transparent );
    // temp_square.setOutlineThickness( 1.f );
    // getWindow().draw( temp_square );
  }
}

void RenderGameSystem::render_armed()
{
  // render armed obstacles with debug outlines
  auto all_armed_obstacles_view = m_reg->view<Cmp::Obstacle, Cmp::Armed, Cmp::Position>();
  for ( auto [entity, obstacle_cmp, armed_cmp, pos_cmp] : all_armed_obstacles_view.each() )
  {
    if ( armed_cmp.m_display_bomb_sprite ) { safe_render_sprite( "BOMB", pos_cmp, 0 ); }

    sf::RectangleShape temp_square( sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } );
    temp_square.setPosition( pos_cmp );
    temp_square.setOutlineColor( sf::Color::Transparent );
    temp_square.setFillColor( sf::Color::Transparent );
    if ( armed_cmp.getElapsedWarningTime() > armed_cmp.m_warning_delay )
    {
      temp_square.setOutlineColor( armed_cmp.m_armed_color );
      temp_square.setFillColor( armed_cmp.m_armed_color );
    }
    temp_square.setOutlineThickness( 1.f );
    getWindow().draw( temp_square );

    // debug - F4
    if ( m_show_armed_obstacles )
    {
      sf::Text text( m_font, "", 12 );
      text.setString( std::to_string( armed_cmp.m_index ) );
      text.setPosition( pos_cmp );
      getWindow().draw( text );
    }
  }
}

void RenderGameSystem::render_loot()
{
  auto loot_view = m_reg->view<Cmp::Obstacle, Cmp::Loot, Cmp::Position>();
  for ( auto [entity, obstacles, loot, position] : loot_view.each() )
  {

    if ( loot.m_type == "EXTRA_HEALTH" ) { safe_render_sprite( "EXTRA_HEALTH", position, loot.m_tile_index ); }
    else if ( loot.m_type == "EXTRA_BOMBS" ) { safe_render_sprite( "EXTRA_BOMBS", position, loot.m_tile_index ); }
    else if ( loot.m_type == "INFINI_BOMBS" ) { safe_render_sprite( "INFINI_BOMBS", position, loot.m_tile_index ); }
    else if ( loot.m_type == "CHAIN_BOMBS" ) { safe_render_sprite( "CHAIN_BOMBS", position, loot.m_tile_index ); }
    else if ( loot.m_type == "LOWER_WATER" ) { safe_render_sprite( "LOWER_WATER", position, loot.m_tile_index ); }
    else
    {
      // Handle unknown loot types
      SPDLOG_WARN( "Unknown loot type: {}", loot.m_type );
    }
  }
}

void RenderGameSystem::render_walls()
{
  // draw walls and door frames
  auto wall_view = m_reg->view<Cmp::Wall, Cmp::Position>();
  for ( auto [entity, wall_cmp, pos_cmp] : wall_view.each() )
  {

    sf::Vector2f new_scale{ 1.f, 1.f };
    uint8_t new_alpha{ 255 };
    sf::Vector2f new_origin{ 0.f, 0.f };
    float angle{ 0.f };

    safe_render_sprite( "WALL", pos_cmp, wall_cmp.m_tile_index, new_scale, new_alpha, new_origin,
                        sf::degrees( angle ) );
  }

  // draw entrance
  auto entrance_door_view = m_reg->view<Cmp::Door, Cmp::Position>( entt::exclude<Cmp::Exit> );
  for ( auto [entity, door_cmp, pos_cmp] : entrance_door_view.each() )
  {

    sf::Vector2f new_scale{ 1.f, 1.f };
    uint8_t new_alpha{ 255 };
    sf::Vector2f new_origin{ 0.f, 0.f };
    float angle{ 0.f };

    safe_render_sprite( "WALL", pos_cmp, door_cmp.m_tile_index, new_scale, new_alpha, new_origin,
                        sf::degrees( angle ) );
  }

  auto exit_door_view = m_reg->view<Cmp::Door, Cmp::Position, Cmp::Exit>();
  for ( auto [entity, door_cmp, pos_cmp, exit_cmp] : exit_door_view.each() )
  {
    auto half_width_px = Sprites::MultiSprite::kDefaultSpriteDimensions.x / 2.f;
    auto half_height_px = Sprites::MultiSprite::kDefaultSpriteDimensions.y / 2.f;

    sf::Vector2f new_pos{ pos_cmp + sf::Vector2f{ half_width_px, half_height_px } };
    sf::Vector2f new_scale{ 1.f, 1.f };
    uint8_t new_alpha{ 255 };
    sf::Vector2f new_origin{ half_width_px, half_height_px };
    float angle{ 180.f };

    safe_render_sprite( "WALL", new_pos, door_cmp.m_tile_index, new_scale, new_alpha, new_origin,
                        sf::degrees( angle ) );
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

    safe_render_sprite( "PLAYER", { pc_pos_cmp.x + dir_cmp.x_offset, pc_pos_cmp.y }, sprite_index );

    if ( m_show_path_distances )
    {
      sf::RectangleShape pc_square( pc_detection_bounds.size() );
      pc_square.setFillColor( sf::Color::Transparent );
      pc_square.setOutlineColor( sf::Color::Green );
      pc_square.setOutlineThickness( 1.f );
      pc_square.setPosition( pc_detection_bounds.position() );
      getWindow().draw( pc_square );
    }

    auto player_hitbox = Cmp::RectBounds( pc_pos_cmp, sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions },
                                          1.2f );
    // // Debug: Draw a green rectangle around the player position
    // sf::RectangleShape player_square( player_hitbox.size() );
    // player_square.setFillColor( sf::Color::Transparent );
    // player_square.setOutlineColor( sf::Color::Green );
    // player_square.setOutlineThickness( 1.f );
    // player_square.setPosition( player_hitbox.position() );
    // getWindow().draw( player_square );
  }
}

void RenderGameSystem::render_player_footsteps()
{
  // add new footstep for player
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction>();
  for ( auto [entity, player, pos_cmp, dir_cmp] : player_view.each() )
  {
    if ( dir_cmp == sf::Vector2f( 0.0f, 0.0f ) ) { continue; }
    m_footstep_sys.add_footstep( pos_cmp, dir_cmp );
  }

  // update all footsteps (fade out and remove if alpha <= 0)
  m_footstep_sys.update();

  // render all footsteps
  auto footstep_view = m_reg->view<Cmp::FootStepTimer, Cmp::FootStepAlpha, Cmp::Position, Cmp::Direction>();
  for ( auto [entity, timer, alpha, position, direction] : footstep_view.each() )
  {
    std::size_t new_idx = 0;
    uint8_t new_alpha{ alpha.m_alpha };
    sf::Vector2f new_scale{ 1.f, 1.f };
    // we're changing the origin to be the center of the sprite so that
    // rotation happens around the center, this means we also need to
    // offset the position to make it look convincing depending on direction of movement
    sf::Vector2f new_origin{ Sprites::MultiSprite::kDefaultSpriteDimensions.x / 2.f,
                             Sprites::MultiSprite::kDefaultSpriteDimensions.y / 2.f };
    sf::Vector2f new_position{ position };
    // moving in right direction: place footsteps to bottom-left of player position
    if ( direction == sf::Vector2f( 1.f, 0.f ) )
    {
      new_position = { position.x + ( Sprites::MultiSprite::kDefaultSpriteDimensions.x * 0.25f ),
                       position.y + ( Sprites::MultiSprite::kDefaultSpriteDimensions.y * 0.75f ) };
    }
    // moving in left direction: place footsteps to bottom-right of player position
    else if ( direction == sf::Vector2f( -1.f, 0.f ) )
    {
      new_position = { position.x + ( Sprites::MultiSprite::kDefaultSpriteDimensions.x * 0.75f ),
                       position.y + ( Sprites::MultiSprite::kDefaultSpriteDimensions.y * 0.75f ) };
    }
    // moving diagonally: down/left
    else if ( direction == sf::Vector2f( -1.f, 1.f ) )
    {
      new_position = { position.x + ( Sprites::MultiSprite::kDefaultSpriteDimensions.x * 0.75f ),
                       position.y + ( Sprites::MultiSprite::kDefaultSpriteDimensions.y * 0.75f ) };
    }
    // moving diagonally: down/right
    else if ( direction == sf::Vector2f( 1.f, 1.f ) )
    {
      new_position = { position.x + ( Sprites::MultiSprite::kDefaultSpriteDimensions.x * 0.5f ),
                       position.y + ( Sprites::MultiSprite::kDefaultSpriteDimensions.y * 0.8f ) };
    }
    // moving diagonally: up/left
    else if ( direction == sf::Vector2f( -1.f, -1.f ) )
    {
      new_position = { position.x + ( Sprites::MultiSprite::kDefaultSpriteDimensions.x * 0.5f ),
                       position.y + ( Sprites::MultiSprite::kDefaultSpriteDimensions.y * 0.8f ) };
    }
    // moving diagonally: up/right
    else if ( direction == sf::Vector2f( 1.f, -1.f ) )
    {
      new_position = { position.x + ( Sprites::MultiSprite::kDefaultSpriteDimensions.x * 0.5f ),
                       position.y + ( Sprites::MultiSprite::kDefaultSpriteDimensions.y * 0.8f ) };
    }
    // moving in up/down direction: place footsteps to center of player position
    else
    {
      new_position = { position.x + ( Sprites::MultiSprite::kDefaultSpriteDimensions.x * 0.5f ),
                       position.y + ( Sprites::MultiSprite::kDefaultSpriteDimensions.y * 0.5f ) };
    }
    // only set rotation if direction is not zero vector
    sf::Angle new_angle;
    if ( direction != sf::Vector2f( 0.f, 0.f ) ) { new_angle = direction.angle(); }

    safe_render_sprite( "FOOTSTEPS", new_position, new_idx, new_scale, new_alpha, new_origin, new_angle );
  }
}

void RenderGameSystem::render_npc()
{
  for ( auto [entity, npc, pos, npc_scan_bounds, direction, anim_cmp] :
        m_reg->view<Cmp::NPC, Cmp::Position, Cmp::NPCScanBounds, Cmp::Direction, Cmp::SpriteAnimation>().each() )
  {
    // flip and x-axis offset the sprite depending on the direction
    if ( direction.x > 0 )
    {
      direction.x_scale = 1.f;
      direction.x_offset = 0.f;
    }
    else if ( direction.x < 0 )
    {
      direction.x_scale = -1.f;
      direction.x_offset = Sprites::MultiSprite::kDefaultSpriteDimensions.x;
    }
    else
    {
      direction.x_scale = direction.x_scale; // keep last known direction
      direction.x_offset = direction.x_offset;
    }

    sf::Vector2f new_scale{ direction.x_scale, 1.f };
    sf::Vector2f new_position{ pos.x + direction.x_offset, pos.y };
    unsigned int new_sprite_idx{ anim_cmp.m_base_frame + anim_cmp.m_current_frame };
    // get the correct sprite index based on animation frame
    safe_render_sprite( "NPC", new_position, new_sprite_idx, new_scale );

    // show npc scan distance
    if ( m_show_path_distances )
    {
      sf::RectangleShape npc_square( npc_scan_bounds.size() );
      npc_square.setFillColor( sf::Color::Transparent );
      npc_square.setOutlineColor( sf::Color::Red );
      npc_square.setOutlineThickness( 1.f );
      npc_square.setPosition( npc_scan_bounds.position() );
      getWindow().draw( npc_square );
    }

    // sf::RectangleShape player_square( sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } );
    // player_square.setFillColor( sf::Color::Transparent );
    // player_square.setOutlineColor( sf::Color::Red );
    // player_square.setOutlineThickness( 1.f );
    // player_square.setPosition( pos );
    // getWindow().draw( player_square );
  }
}

void RenderGameSystem::render_explosions()
{

  auto explosion_view = m_reg->view<Cmp::NpcDeathPosition, Cmp::SpriteAnimation>();
  for ( auto [entity, pos_cmp, anim_cmp] : explosion_view.each() )
  {

    // Always render the current frame
    SPDLOG_DEBUG( "Rendering explosion frame {}/{} for entity {}", anim_cmp.m_current_frame,
                  m_explosion_ms->get_sprites_per_sequence(), static_cast<int>( entity ) );

    safe_render_sprite( "EXPLOSION", pos_cmp, anim_cmp.m_current_frame );
  }
}

void RenderGameSystem::render_flood_waters()
{
  for ( auto [_, _wl] : m_reg->view<Cmp::WaterLevel>().each() )
  {
    if ( _wl.m_level > 0 ) { m_water_shader.update( _wl.m_level ); }
    getWindow().draw( m_water_shader );
  }
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
  //     // getWindow().draw(distance_text);

  // }
}

void RenderGameSystem::render_player_distances_on_obstacles()
{
  if ( !m_show_path_distances ) return;
  auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position, Cmp::PlayerDistance>();
  for ( auto [_ob_entt, _ob, _ob_pos, _player_distance] : obstacle_view.each() )
  {
    sf::Text distance_text( m_font, "", 10 );
    distance_text.setString( std::to_string( _player_distance.distance ) );
    distance_text.setPosition( _ob_pos + sf::Vector2f{ 5.f, 0.f } );
    distance_text.setFillColor( sf::Color::White );
    distance_text.setOutlineColor( sf::Color::Black );
    distance_text.setOutlineThickness( 2.f );
    getWindow().draw( distance_text );
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

      distance_text.setPosition( *obstacle_position );
      distance_text.setFillColor( sf::Color::White );
      distance_text.setOutlineColor( sf::Color::Black );
      distance_text.setOutlineThickness( 2.f );
      getWindow().draw( distance_text );
    }
  }
}

void RenderGameSystem::safe_render_sprite_to_target( sf::RenderTarget &target, const std::string &sprite_type,
                                                     const sf::Vector2f &position, int sprite_index, sf::Vector2f scale,
                                                     uint8_t alpha, sf::Vector2f origin, sf::Angle angle )
{
  try
  {
    auto &sprite = m_multisprite_map.at( sprite_type );
    if ( sprite.has_value() )
    {
      auto pick_result = sprite->pick( sprite_index, sprite_type );
      sprite->setPosition( position );
      sprite->setScale( scale );
      sprite->set_pick_opacity( alpha );
      sprite->setOrigin( origin );
      sprite->setRotation( angle );
      if ( pick_result )
      {
        target.draw( *sprite ); // Draw to specified target instead of getWindow()
      }
      else { render_fallback_square_to_target( target, position, sf::Color::Cyan ); }
    }
    else
    {
      // SPDLOG_WARN( "Sprite '{}' exists in map but has no value", sprite_type );
      render_fallback_square_to_target( target, position, sf::Color::Yellow );
    }
  }
  catch ( const std::out_of_range &e )
  {
    // SPDLOG_WARN( "Missing sprite '{}' in map, rendering fallback square", sprite_type );
    render_fallback_square_to_target( target, position, sf::Color::Magenta );
  }
}

void RenderGameSystem::render_fallback_square_to_target( sf::RenderTarget &target, const sf::Vector2f &position,
                                                         const sf::Color &color )
{
  sf::RectangleShape fallback_square( sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } );
  fallback_square.setPosition( position );
  fallback_square.setFillColor( color );
  fallback_square.setOutlineColor( sf::Color::White );
  fallback_square.setOutlineThickness( 1.f );
  target.draw( fallback_square ); // Draw to specified target
}

// Keep the original for backwards compatibility
void RenderGameSystem::safe_render_sprite( const std::string &sprite_type, const sf::Vector2f &position,
                                           int sprite_index, sf::Vector2f scale, uint8_t alpha, sf::Vector2f origin,
                                           sf::Angle angle )
{
  safe_render_sprite_to_target( getWindow(), sprite_type, position, sprite_index, scale, alpha, origin, angle );
}

void RenderGameSystem::render_fallback_square( const sf::Vector2f &position, const sf::Color &color )
{
  render_fallback_square_to_target( getWindow(), position, color );
}

void RenderGameSystem::update_view_center( sf::View &view, const Cmp::Position &player_pos, float smoothFactor )
{
  const float kHalfViewWidth = view.getSize().x * 0.5f;
  const float kHalfViewHeight = view.getSize().y * 0.5f;

  // Calculate the maximum allowed camera positions
  float maxX = kDisplaySize.x - kHalfViewWidth;
  float maxY = kDisplaySize.y - kHalfViewHeight;

  // Calculate new camera position
  float newX = std::clamp( player_pos.x, kHalfViewWidth, maxX );
  float newY = std::clamp( player_pos.y, kHalfViewHeight, maxY );

  // Smoothly interpolate to the new position
  sf::Vector2f currentCenter = view.getCenter();

  view.setCenter( { currentCenter.x + ( newX - currentCenter.x ) * smoothFactor,
                    currentCenter.y + ( newY - currentCenter.y ) * smoothFactor } );
}

} // namespace ProceduralMaze::Sys