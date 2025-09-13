#include <GameState.hpp>
#include <NpcDeathPosition.hpp>
#include <Position.hpp>
#include <RenderSystem.hpp>
#include <Systems/RenderGameSystem.hpp>
#include <string>

namespace ProceduralMaze::Sys {

RenderGameSystem::RenderGameSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg ) : RenderSystem( reg )
{

  // init local view dimensions
  m_local_view = sf::View( { LOCAL_MAP_VIEW_SIZE.x * 0.5f, LOCAL_MAP_VIEW_SIZE.y * 0.5f }, LOCAL_MAP_VIEW_SIZE );
  m_local_view.setViewport( sf::FloatRect( { 0.f, 0.f }, { 1.f, 1.f } ) );

  // init minimap view dimensions
  m_minimap_view = sf::View( { MINI_MAP_VIEW_SIZE.x * 0.5f, MINI_MAP_VIEW_SIZE.y * 0.5f }, MINI_MAP_VIEW_SIZE );
  m_minimap_view.setViewport( sf::FloatRect( { 0.75f, 0.f }, { 0.25f, 0.25f } ) );
}

void RenderGameSystem::render_game( sf::Time deltaTime )
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
      render_floormap( { 0, kMapGridOffset.y * Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.y } );
      render_obstacles();
      render_armed();
      render_loot();
      render_walls();
      render_player();
      render_npc();
      render_explosions( deltaTime );
      render_flood_waters();
      render_player_distances_on_npc();
      // render_npc_distances_on_obstacles();
      render_player_distances_on_obstacles();

      // move the local view position to equal the player position
      // reset the center if player is stuck
      for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
      {
        if ( _sys.player_stuck )
        {
          m_local_view.setCenter( { LOCAL_MAP_VIEW_SIZE.x * 0.5f, kDisplaySize.y * 0.5f } );
          _sys.player_stuck = false;
        }
        else
        {
          for ( auto [entity, _pc, _pos] : m_reg->view<Cmp::PlayableCharacter, Cmp::Position>().each() )
          {
            update_view_center( m_local_view, _pos );
          }
        }
      }
    }
    // local view end

    // minimap view begin - this show a quarter of the game world but in a
    // much smaller scale
    getWindow().setView( m_minimap_view );
    {
      render_floormap( { 0, kMapGridOffset.y * Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.y } );
      render_obstacles();
      render_armed();
      render_loot();
      render_walls();
      render_player();
      render_npc();
      render_flood_waters();

      // update the minimap view center based on player position
      // reset the center if player is stuck
      for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
      {
        if ( _sys.player_stuck )
        {
          m_minimap_view.setCenter( { MINI_MAP_VIEW_SIZE.x * 0.5f, MINI_MAP_VIEW_SIZE.y * 0.5f } );
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
      auto minimap_border = sf::RectangleShape( { MINI_MAP_VIEW_SIZE.x, MINI_MAP_VIEW_SIZE.y } );
      minimap_border.setPosition( { kDisplaySize.x - MINI_MAP_VIEW_SIZE.x, 0.f } );
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

void RenderGameSystem::render_obstacles()
{

  // Group similar draw operations to reduce state changes
  std::vector<std::pair<sf::Vector2f, int>> rockPositions;
  std::vector<std::pair<sf::Vector2f, int>> potPositions;
  std::vector<std::pair<sf::Vector2f, int>> bonePositions;
  std::vector<std::pair<sf::Vector2f, int>> npcPositions;
  std::vector<std::pair<sf::Vector2f, Sprites::SpriteFactory::SpriteMetaType>> disabledPositions;
  std::vector<sf::Vector2f> detonationPositions;

  // Collect all positions first instead of drawing immediately
  for ( auto [entity, _ob, _pos, _ob_nb_list] : m_reg->view<Cmp::Obstacle, Cmp::Position, Cmp::Neighbours>().each() )
  {

    if ( _ob.m_enabled )
    {
      switch ( _ob.m_type )
      {
      case Sprites::SpriteFactory::SpriteMetaType::ROCK:
        rockPositions.emplace_back( _pos, _ob.m_tile_index );
        break;
      case Sprites::SpriteFactory::SpriteMetaType::POT:
        potPositions.emplace_back( _pos, _ob.m_tile_index );
        break;
      case Sprites::SpriteFactory::SpriteMetaType::BONES:
        bonePositions.emplace_back( _pos, _ob.m_tile_index );
        break;
      default:
        break;
      }
    }
    // else {
    //     disabledPositions.emplace_back(_pos, _ob.m_type);
    // }
    if ( _ob.m_broken ) { detonationPositions.push_back( _pos ); }
  }

  // Now draw each type in batches
  for ( const auto &[pos, idx] : rockPositions )
  {
    m_rock_ms->pick( idx, "Obstacle" );
    m_rock_ms->setPosition( pos );
    getWindow().draw( *m_rock_ms );
  }

  for ( const auto &[pos, idx] : potPositions )
  {
    m_pot_ms->pick( idx, "Obstacle" );
    m_pot_ms->setPosition( pos );
    getWindow().draw( *m_pot_ms );
  }

  for ( const auto &[pos, idx] : bonePositions )
  {
    m_bone_ms->pick( idx, "Obstacle" );
    m_bone_ms->setPosition( pos );
    getWindow().draw( *m_bone_ms );
  }

  // "empty" sprite for detonated objects
  for ( const auto &pos : detonationPositions )
  {
    m_detonation_ms->setPosition( pos );
    m_detonation_ms->pick( 0, "Detonated" );
    getWindow().draw( *m_detonation_ms );
  }

  // for(const auto& [pos, type]: disabledPositions) {
  //     sf::Text text(m_font, "", 12);
  //     text.setString(m_sprite_factory->get_metadata_type_string(type));
  //     text.setPosition(pos);
  //     getWindow().draw(text);

  //     sf::RectangleShape
  //     temp_square(sf::Vector2f{Sprites::MultiSprite::DEFAULT_SPRITE_SIZE});
  //     temp_square.setPosition(pos);
  //     temp_square.setFillColor(sf::Color::Transparent);
  //     temp_square.setOutlineColor(sf::Color::Red);
  //     temp_square.setOutlineThickness(1.f);
  //     getWindow().draw(temp_square);
  // }
}

void RenderGameSystem::render_armed()
{
  // render armed obstacles with debug outlines
  auto all_armed_obstacles_view = m_reg->view<Cmp::Obstacle, Cmp::Armed, Cmp::Position>();
  for ( auto [entity, obstacle_cmp, armed_cmp, pos_cmp] : all_armed_obstacles_view.each() )
  {
    if ( armed_cmp.m_display_bomb_sprite )
    {
      m_bomb_ms->pick( 0, "Bomb" );
      m_bomb_ms->setPosition( pos_cmp );
      getWindow().draw( *m_bomb_ms );
    }

    sf::RectangleShape temp_square( sf::Vector2f{ Sprites::MultiSprite::DEFAULT_SPRITE_SIZE } );
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
    switch ( loot.m_type )
    {
    case ProceduralMaze::Sprites::SpriteFactory::SpriteMetaType::EXTRA_HEALTH:
      m_extra_health_ms->setPosition( position );
      m_extra_health_ms->pick( loot.m_tile_index, "EXTRA_HEALTH" );
      getWindow().draw( *m_extra_health_ms );
      break;
    case ProceduralMaze::Sprites::SpriteFactory::SpriteMetaType::EXTRA_BOMBS:
      m_extra_bombs_ms->setPosition( position );
      m_extra_bombs_ms->pick( loot.m_tile_index, "EXTRA_BOMBS" );
      getWindow().draw( *m_extra_bombs_ms );
      break;
    case ProceduralMaze::Sprites::SpriteFactory::SpriteMetaType::INFINI_BOMBS:
      m_infinite_bombs_ms->setPosition( position );
      m_infinite_bombs_ms->pick( loot.m_tile_index, "INFINI_BOMBS" );
      getWindow().draw( *m_infinite_bombs_ms );
      break;
    case ProceduralMaze::Sprites::SpriteFactory::SpriteMetaType::CHAIN_BOMBS:
      m_chain_bombs_ms->setPosition( position );
      m_chain_bombs_ms->pick( loot.m_tile_index, "CHAIN_BOMBS" );
      getWindow().draw( *m_chain_bombs_ms );
      break;
    case ProceduralMaze::Sprites::SpriteFactory::SpriteMetaType::LOWER_WATER:
      m_lower_water_ms->setPosition( position );
      m_lower_water_ms->pick( loot.m_tile_index, "LOWER_WATER" );
      getWindow().draw( *m_lower_water_ms );
      break;
    default:
      break;
    }
  }
}

void RenderGameSystem::render_walls()
{
  // Render textures for "WALL" entities - filtered out because they don't own
  // neighbour components
  for ( auto [entity, _ob, _pos] : m_reg->view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::Neighbours> ).each() )
  {

    m_wall_ms->pick( _ob.m_tile_index, "wall" );
    m_wall_ms->setPosition( _pos );
    getWindow().draw( *m_wall_ms );
  }
}

void RenderGameSystem::render_player()
{
  for ( auto [entity, player, position, direction, pc_detection_bounds] :
        m_reg->view<Cmp::PlayableCharacter, Cmp::Position, Cmp::Direction, Cmp::PCDetectionBounds>().each() )
  {

    // flip and x-axis offset the sprite depending on the direction
    if ( direction.x == 1 )
    {
      direction.x_scale = 1.f;
      direction.x_offset = 0.f;
    }
    else if ( direction.x == -1 )
    {
      direction.x_scale = -1.f;
      direction.x_offset = Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.x;
    }
    else
    {
      direction.x_scale = direction.x_scale; // keep last known direction
      direction.x_offset = direction.x_offset;
    }

    m_player_ms->setScale( { direction.x_scale, 1.f } );
    m_player_ms->setPosition( { position.x + direction.x_offset, position.y } );

    m_player_ms->pick( 0, "player" );
    getWindow().draw( *m_player_ms );
    if ( m_show_path_distances )
    {
      sf::RectangleShape pc_square( pc_detection_bounds.size() );
      pc_square.setFillColor( sf::Color::Transparent );
      pc_square.setOutlineColor( sf::Color::Green );
      pc_square.setOutlineThickness( 1.f );
      pc_square.setPosition( pc_detection_bounds.position() );
      getWindow().draw( pc_square );
    }
  }
}

void RenderGameSystem::render_npc()
{
  for ( auto [entity, npc, pos, npc_scan_bounds, direction] : m_reg->view<Cmp::NPC, Cmp::Position, Cmp::NPCScanBounds, Cmp::Direction>().each() )
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
      direction.x_offset = Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.x;
    }
    else
    {
      direction.x_scale = direction.x_scale; // keep last known direction
      direction.x_offset = direction.x_offset;
    }

    m_npc_ms->setScale( { direction.x_scale, 1.f } );
    m_npc_ms->setPosition( { pos.x + direction.x_offset, pos.y } );

    m_npc_ms->pick( 2, "npc" );
    getWindow().draw( *m_npc_ms );

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
  }
}

void RenderGameSystem::render_explosions( sf::Time deltaTime )
{
  // TODO possibly add a persistent component here so it can be set ingame
  const sf::Time kAnimFrameRate = sf::seconds( 0.05f );

  auto explosion_view = m_reg->view<Cmp::NpcDeathPosition>();
  for ( auto [entity, explosion_cmp] : explosion_view.each() )
  {
    SPDLOG_DEBUG( "Rendering {} active explosions", explosion_view.size() );

    auto max_anim_frame = m_explosion_ms->get_sprite_count();
    // have we completed the animation?
    if ( explosion_cmp.current_anim_frame >= max_anim_frame )
    {
      m_reg->remove<Cmp::NpcDeathPosition>( entity );
      SPDLOG_DEBUG( "Explosion animation complete, removing component from entity {}", static_cast<int>( entity ) );
      continue;
    }

    // Always render the current frame
    SPDLOG_DEBUG( "Rendering explosion frame {}/{} for entity {}", explosion_cmp.current_anim_frame, max_anim_frame, static_cast<int>( entity ) );
    m_explosion_ms->pick( explosion_cmp.current_anim_frame, "explosion" );
    m_explosion_ms->setPosition( explosion_cmp );
    getWindow().draw( *m_explosion_ms );

    // Accumulate time and advance frame when threshold is reached
    explosion_cmp.elapsed_time += deltaTime;
    if ( explosion_cmp.elapsed_time >= kAnimFrameRate )
    {
      explosion_cmp.current_anim_frame++;
      explosion_cmp.elapsed_time = sf::Time::Zero; // Reset timer
    }
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

void RenderGameSystem::load_multisprites()
{
  using namespace Sprites;
  auto &factory = m_reg->ctx().get<std::shared_ptr<SpriteFactory>>();
  m_rock_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::ROCK );
  m_pot_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::POT );
  m_bone_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::BONES );
  m_detonation_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::DETONATED );
  m_bomb_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::BOMB );
  m_wall_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::WALL );
  m_player_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::PLAYER );
  m_npc_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::NPC );
  m_extra_health_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::EXTRA_HEALTH );
  m_extra_bombs_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::EXTRA_BOMBS );
  m_infinite_bombs_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::INFINI_BOMBS );
  m_chain_bombs_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::CHAIN_BOMBS );
  m_lower_water_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::LOWER_WATER );
  m_explosion_ms = factory->get_multisprite_by_type( SpriteFactory::SpriteMetaType::EXPLOSION );

  // we should ensure these MultiSprites are initialized before continuing
  std::string err_msg;
  if ( !m_rock_ms ) { err_msg = "Unable to get ROCK from SpriteFactory"; }
  if ( !m_pot_ms ) { err_msg = "Unable to get POT from SpriteFactory"; }
  if ( !m_bone_ms ) { err_msg = "Unable to get BONE from SpriteFactory"; }
  if ( !m_detonation_ms ) { err_msg = "Unable to get DETONATION from SpriteFactory"; }
  if ( !m_bomb_ms ) { err_msg = "Unable to get BOMB from SpriteFactory"; }
  if ( !m_wall_ms ) { err_msg = "Unable to get WALL from SpriteFactory"; }
  if ( !m_player_ms ) { err_msg = "Unable to get PLAYER from SpriteFactory"; }
  if ( !m_npc_ms ) { err_msg = "Unable to get NPC from SpriteFactory"; }
  if ( !m_extra_health_ms ) { err_msg = "Unable to get EXTRA_HEALTH from SpriteFactory"; }
  if ( !m_extra_bombs_ms ) { err_msg = "Unable to get EXTRA_BOMBS from SpriteFactory"; }
  if ( !m_infinite_bombs_ms ) { err_msg = "Unable to get INFINI_BOMBS from SpriteFactory"; }
  if ( !m_chain_bombs_ms ) { err_msg = "Unable to get CHAIN_BOMBS from SpriteFactory"; }
  if ( !m_lower_water_ms ) { err_msg = "Unable to get LOWER_WATER from SpriteFactory"; }
  if ( !m_explosion_ms ) { err_msg = "Unable to get EXPLOSION from SpriteFactory"; }

  if ( !err_msg.empty() )
  {
    SPDLOG_CRITICAL( "{}", err_msg );
    std::get_terminate();
  }
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

  view.setCenter( { currentCenter.x + ( newX - currentCenter.x ) * smoothFactor, currentCenter.y + ( newY - currentCenter.y ) * smoothFactor } );
}
}; // namespace ProceduralMaze::Sys