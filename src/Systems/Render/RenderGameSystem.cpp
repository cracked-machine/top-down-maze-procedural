#include <Components/Armed.hpp>
#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptInteriorMultiBlock.hpp>
#include <Components/Crypt/CryptLever.hpp>
#include <Components/Crypt/CryptObjectiveMultiBlock.hpp>
#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Crypt/CryptPassageSpikeTrap.hpp>
#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomLavaPit.hpp>
#include <Components/Crypt/CryptRoomLavaPitCell.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Inventory/ScryingBall.hpp>
#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Persistent/CameraSmoothSpeed.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/Player/PlayerBlastRadius.hpp>
#include <Components/Player/PlayerCadaverCount.hpp>
#include <Components/Player/PlayerWealth.hpp>
#include <Components/Ruin/RuinBuildingMultiBlock.hpp>
#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteOffset.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Crypt/CryptEntrance.hpp>
#include <Components/Crypt/CryptMultiBlock.hpp>
#include <Components/Crypt/CryptSegment.hpp>
#include <Components/DeathPosition.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Grave/GraveMultiBlock.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/Hazard/HazardFieldCell.hpp>
#include <Components/Hazard/SinkholeCell.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/LootContainer.hpp>
#include <Components/Npc/NpcContainer.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Components/Wormhole/WormholeMultiBlock.hpp>
#include <Components/Wormhole/WormholeSingularity.hpp>
#include <Components/ZOrderValue.hpp>
#include <Systems/Threats/HazardFieldSystemImpl.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Utils.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <queue>

namespace ProceduralMaze::Sys
{

RenderGameSystem::RenderGameSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                    Audio::SoundBank &sound_bank )
    : RenderSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "RenderGameSystem initialized" );
}

void RenderGameSystem::refresh_z_order_queue()
{
  m_zorder_queue_.clear();
  sf::FloatRect view_bounds = Utils::calculate_view_bounds( m_local_view );

  // prevent pop-in/pop-outs when multiblock entities are near the edge of the view
  add_visible_entity_to_z_order_queue<Cmp::AltarMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::CryptMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::GraveMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::HolyWellMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::CryptInteriorMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::RuinBuildingMultiBlock>( m_zorder_queue_, view_bounds );

  // add other components as normal
  add_visible_entity_to_z_order_queue<Cmp::Position>( m_zorder_queue_, view_bounds );

  std::sort( m_zorder_queue_.begin(), m_zorder_queue_.end(), []( const ZOrder &a, const ZOrder &b ) { return a.z < b.z; } );
}

void RenderGameSystem::init_views()
{
  // init local view dimensions
  m_local_view = sf::View( { kLocalMapViewSizeF.x * 0.5f, kLocalMapViewSizeF.y * 0.5f }, kLocalMapViewSizeF );
  m_local_view.setViewport( sf::FloatRect( { 0.f, 0.f }, { 1.f, 1.f } ) );

  auto start_pos = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerStartPosition>( getReg() );
  m_local_view.setCenter( start_pos );
}

void RenderGameSystem::init_shaders()
{
  auto display_res = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DisplayResolution>( getReg() );
  m_water_shader.resize_texture( display_res );
  m_water_shader.setup();

  m_wormhole_shader.setup();

  m_pulsing_shader.resize_texture( display_res );
  m_pulsing_shader.setup();

  m_mist_shader.resize_texture( display_res );
  m_mist_shader.setup();

  m_dark_mode_shader.resize_texture( display_res );
  m_dark_mode_shader.setup();
}

void RenderGameSystem::updateCamera( sf::Time deltaTime )
{

  // Get the player's current position
  auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
  for ( auto [entity, pc_cmp, pos_cmp] : player_view.each() )
  {
    sf::Vector2f target_position = pos_cmp.position;

    // Initialize camera position on first frame to avoid lerping from origin
    if ( !m_camera_initialized )
    {
      m_camera_position = target_position;
      m_camera_initialized = true;
    }

    // Smooth lerp toward target position
    float dt = deltaTime.asSeconds();
    auto camera_smooth_speed = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::CameraSmoothSpeed>( getReg() ).get_value();
    float t = 1.0f - std::exp( -camera_smooth_speed * dt ); // Exponential smoothing

    m_camera_position.x += ( target_position.x - m_camera_position.x ) * t;
    m_camera_position.y += ( target_position.y - m_camera_position.y ) * t;

    // Snap to target if very close (prevents endless micro-adjustments)
    constexpr float kSnapThreshold = 0.5f;
    if ( std::abs( target_position.x - m_camera_position.x ) < kSnapThreshold &&
         std::abs( target_position.y - m_camera_position.y ) < kSnapThreshold )
    {
      m_camera_position = target_position;
    }

    // Update the view center
    m_local_view.setCenter( m_camera_position + ( pos_cmp.size / 2.f ) ); // Center on sprite center
  }
}

void RenderGameSystem::render_game( [[maybe_unused]] sf::Time globalDeltaTime, RenderOverlaySystem &render_overlay_sys,
                                    Sprites::Containers::TileMap &floormap, DarkMode dark_mode, WeatherMode weather_mode )
{
  using namespace Sprites;

  // check for updates to the System modes
  for ( auto [_ent, _sys] : getReg().view<Cmp::System>().each() )
  {
    m_show_path_finding = _sys.show_path_distances;
    m_show_armed_obstacles = _sys.show_armed_obstacles;
    m_minimap_enabled = _sys.minimap_enabled;
    m_show_debug_stats = _sys.show_debug_stats;
    m_render_dark_mode_enabled = _sys.dark_mode_enabled;
  }

  sf::FloatRect player_position( { 0.f, 0.f }, Constants::kGridSquareSizePixelsF );
  for ( auto [entity, pc_cmp, pc_pos_cmp] : getReg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
  {
    player_position = pc_pos_cmp;
  }

  // make sure the local view is centered on the player mid-point and not at their top-left corner
  // (otherwise this makes views, shaders, etc look off-center)
  // m_local_view.setCenter( player_position.position + Constants::kGridSquareSizePixelsF * 0.5f );
  updateCamera( globalDeltaTime );

  // re-populate the z-order queue with the latest entity/component data
  refresh_z_order_queue();

  // main render begin
  m_window.clear();
  {
    // local view begin - this shows only a `LOCAL_MAP_VIEW_SIZE` of the game world
    m_window.setView( m_local_view );
    {
      // update the static game view reference
      RenderSystem::s_game_view = m_local_view;

      render_background_water( player_position );
      render_floormap( floormap, { 0, 0 } );
      render_scryingball_doglegs();

      for ( const auto &zorder_entry : m_zorder_queue_ )
      {
        auto entity = zorder_entry.e;
        if ( getReg().all_of<Cmp::Position, Cmp::SpriteAnimation>( entity ) )
        {
          const auto &pos_cmp = getReg().get<Cmp::Position>( entity );
          const auto &anim_cmp = getReg().get<Cmp::SpriteAnimation>( entity );

          uint8_t alpha_value = 255;
          auto obst_cmp = getReg().try_get<Cmp::AbsoluteAlpha>( entity );
          if ( obst_cmp ) alpha_value = static_cast<uint8_t>( obst_cmp->getAlpha() );

          sf::Vector2f new_origin_value = { 0.F, 0.F };
          auto new_offset_cmp = getReg().try_get<Cmp::AbsoluteOffset>( entity );
          if ( new_offset_cmp ) new_origin_value = new_offset_cmp->getOffset();

          sf::Angle new_angle_value = sf::degrees( 0.f );
          auto new_angle_cmp = getReg().try_get<Cmp::AbsoluteRotation>( entity );
          if ( new_angle_cmp ) new_angle_value = sf::degrees( new_angle_cmp->getAngle() );

          safe_render_sprite( anim_cmp.m_sprite_type, pos_cmp, anim_cmp.getFrameIndexOffset() + anim_cmp.m_current_frame, { 1.f, 1.f }, alpha_value,
                              new_origin_value, new_angle_value );

          if ( getReg().any_of<Cmp::InventoryWearLevel>( entity ) )
          {
            render_overlay_sys.render_wear_level( getReg().get<Cmp::InventoryWearLevel>( entity ), pos_cmp );
          }
        }
      }

      render_armed();
      render_shockwaves( floormap );
      render_arrow_compass();
      if ( weather_mode == WeatherMode::ON ) { render_mist( player_position ); }
      // lava pit outline
      render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomLavaPit>( sf::Color( 64, 64, 64 ), 0.5f );

      if ( dark_mode == DarkMode::ON && m_render_dark_mode_enabled ) { render_dark_mode_shader(); }

      // debug: show crypt component boundaries
      if ( m_show_debug_stats )
      {
        for ( auto [ruin_entt, access_cmp] : getReg().view<Cmp::RuinFloorAccess>().each() )
        {
          sf::RectangleShape rectangle;
          rectangle.setSize( access_cmp.size );
          rectangle.setPosition( access_cmp.position );
          rectangle.setFillColor( sf::Color::Transparent );
          rectangle.setOutlineThickness( 1.f );
          if ( access_cmp.m_direction == Cmp::RuinFloorAccess::Direction::TO_LOWER ) { rectangle.setOutlineColor( sf::Color::Blue ); }
          else { rectangle.setOutlineColor( sf::Color::White ); }
          m_window.draw( rectangle );
        }

        render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomLavaPitCell>( sf::Color( 254, 128, 32 ), 0.5f );
        render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomOpen>( sf::Color::Green, 1.f );
        render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomStart>( sf::Color::Blue, 1.f );
        render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomEnd>( sf::Color::Yellow, 1.f );
        render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomClosed>( sf::Color::Red, 1.f );
        render_overlay_sys.render_square_for_vector2f_cmp<Cmp::CryptPassageBlock>( sf::Color::Cyan, 1.f );

        for ( auto [lever_entt, lever_cmp, lever_pos_cmp] : getReg().view<Cmp::CryptLever, Cmp::Position>().each() )
        {
          Cmp::RectBounds expand_lever_pos_hitbox( lever_pos_cmp.position, lever_pos_cmp.size, 1.f );
          sf::RectangleShape rectangle;
          rectangle.setSize( expand_lever_pos_hitbox.size() );
          rectangle.setPosition( expand_lever_pos_hitbox.position() );
          rectangle.setFillColor( sf::Color::Transparent );
          rectangle.setOutlineColor( sf::Color::White );
          rectangle.setOutlineThickness( 1.f );
          m_window.draw( rectangle );
        }

        for ( auto [chest_entt, chest_cmp, chest_pos_cmp] : getReg().view<Cmp::CryptChest, Cmp::Position>().each() )
        {
          Cmp::RectBounds expand_lever_pos_hitbox( chest_pos_cmp.position, chest_pos_cmp.size, 1.f );
          sf::RectangleShape rectangle;
          rectangle.setSize( expand_lever_pos_hitbox.size() );
          rectangle.setPosition( expand_lever_pos_hitbox.position() );
          rectangle.setFillColor( sf::Color::Transparent );
          rectangle.setOutlineColor( sf::Color::Magenta );
          rectangle.setOutlineThickness( 1.f );
          m_window.draw( rectangle );
        }
      }
    }
    // local view end

    // UI Overlays begin (these will always be displayed no matter where the player moves)
    m_window.setView( m_window.getDefaultView() );
    {

      // init metrics

      int new_weapon_level = 0;
      int player_cadaver_count = 0;

      sf::Vector2i mouse_pixel_pos = sf::Mouse::getPosition( m_window );
      sf::Vector2f mouse_world_pos = m_window.mapPixelToCoords( mouse_pixel_pos, RenderSystem::getGameView() );

      auto player_blast_radius = Utils::get_player_blast_radius( getReg() );
      auto player_health = Utils::get_player_health( getReg() );
      auto player_wealth = Utils::get_player_wealth( getReg() );

      auto inventory_wear_view = getReg().view<Cmp::PlayerInventorySlot, Cmp::InventoryWearLevel>();
      for ( auto [weapons_entity, inventory_slot, wear_level] : inventory_wear_view.each() )
      {
        new_weapon_level = wear_level.m_level;
      }

      for ( auto [entity, cadaver_cmp] : getReg().view<Cmp::PlayerCadaverCount>().each() )
      {
        player_cadaver_count = cadaver_cmp.get_count();
      }

      // render metrics
      float start_y_pos = 0;
      render_overlay_sys.render_ui_background_overlay( { 20.f, start_y_pos += 20.f }, { 300.f, 230.f } );
      render_overlay_sys.render_health_overlay( player_health.health, { 40.f, start_y_pos += 20.f }, { 200.f, 20.f } );
      render_overlay_sys.render_weapons_meter_overlay( new_weapon_level, { 40.f, start_y_pos += 40.f }, { 200.f, 20.f } );
      render_overlay_sys.render_bomb_overlay( player_blast_radius.value, { 40.f, start_y_pos += 40.f } );
      render_overlay_sys.render_cadaver_count_overlay( player_cadaver_count, { 40.f, start_y_pos += 40.f } );
      render_overlay_sys.render_wealth_overlay( player_wealth.wealth, { 40.f, start_y_pos += 40.f } );

      render_overlay_sys.render_inventory_overlay( { 40.f, start_y_pos += 80.f } );

      auto display_size = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DisplayResolution>( getReg() );
      render_overlay_sys.render_crypt_maze_timer( { static_cast<float>( display_size.x / 2.f ), static_cast<float>( 0 ) }, 100 );

      if ( m_show_debug_stats )
      {

        render_overlay_sys.render_player_position_overlay( player_position.position, { 40.f, start_y_pos += 80.f } );
        render_overlay_sys.render_mouse_position_overlay( mouse_world_pos, { 40.f, start_y_pos += 40.f } );

        sf::Vector2f stats_pos1{ 40.f, start_y_pos += 40.f };
        sf::Vector2f stats_pos2{ 40.f, start_y_pos += 40.f };
        render_overlay_sys.render_stats_overlay( stats_pos1, stats_pos2 );

        render_overlay_sys.render_zorder_values_overlay(
            { display_size.x - 800.f, 40.f }, m_zorder_queue_,
            { "ROCK", "CRYPT.interior_sb", "WALL", "PLAYERSPAWN", "NPCSKELE", "NPCGHOST", "DETONATED" } );
        sf::Vector2u display_size = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DisplayResolution>( getReg() );
        render_overlay_sys.render_npc_list_overlay( { display_size.x - 600.f, 200.f } );
      }
      if ( m_show_path_finding )
      {
        // Save the current view
        sf::View previous_view = m_window.getView();
        // Set the game view for world-space rendering
        m_window.setView( RenderSystem::s_game_view );

        render_overlay_sys.render_scan_detection_bounds();
        render_overlay_sys.render_player_distances();
        render_overlay_sys.render_lerp_positions();

        // Restore the previous view
        m_window.setView( previous_view );
      }
      if ( m_show_armed_obstacles )
      {
        // Save the current view
        sf::View previous_view = m_window.getView();
        // Set the game view for world-space rendering
        m_window.setView( RenderSystem::s_game_view );

        render_overlay_sys.render_obstacle_markers();

        // Restore the previous view
        m_window.setView( previous_view );
      }
    }
    // UI Overlays end
  }

  m_window.display();
  // main render end
}

void RenderGameSystem::render_floormap( Sprites::Containers::TileMap &floormap, const sf::Vector2f &offset )
{
  floormap.setPosition( offset );
  m_window.draw( floormap );
}

void RenderGameSystem::render_armed()
{
  // render armed obstacles with debug outlines
  auto armed_view = getReg().view<Cmp::Armed, Cmp::Position>();
  for ( auto [entity, armed_cmp, pos_cmp] : armed_view.each() )
  {
    if ( armed_cmp.m_display_bomb_sprite ) { safe_render_sprite( "CARRYITEM.bomb", pos_cmp, 0 ); }

    sf::RectangleShape temp_square( Constants::kGridSquareSizePixelsF );
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
  }
}

void RenderGameSystem::render_shockwaves( [[maybe_unused]] Sprites::Containers::TileMap &floormap )
{
  for ( auto [npc_sh_entt, npc_sw_cmp] : getReg().view<Cmp::NpcShockwave>().each() )
  {
    for ( const auto &segment : npc_sw_cmp.sprite.getVisibleSegments() )
    {
      sf::FloatRect segment_bounds = segment.getBounds( npc_sw_cmp.sprite.getPosition(), npc_sw_cmp.sprite.getRadius(),
                                                        npc_sw_cmp.sprite.getOutlineThickness() );

      if ( Utils::is_visible_in_view( RenderSystem::getGameView(), segment_bounds ) )
      {
        // m_shockwave_shader.update_shader_position( segment_bounds.position, Sprites::ViewFragmentShader::Align::TOPLEFT );
        // // m_shockwave_shader.resize_texture( sf::Vector2u{ segment_bounds.size } );
        // // floormap.draw( m_shockwave_shader.get_render_texture(), sf::RenderStates::Default );
        // segment.draw( m_shockwave_shader.get_render_texture(), sf::RenderStates::Default, npc_sw_cmp.sprite.getPosition(),
        //               npc_sw_cmp.sprite.getRadius(), npc_sw_cmp.sprite.getOutlineThickness(), npc_sw_cmp.sprite.getOutlineColor(),
        //               npc_sw_cmp.sprite.getPointsPerSegment() );

        // m_window.draw( m_shockwave_shader );

        segment.draw( m_window, sf::RenderStates::Default, npc_sw_cmp.sprite.getPosition(), npc_sw_cmp.sprite.getRadius(),
                      npc_sw_cmp.sprite.getOutlineThickness(), npc_sw_cmp.sprite.getOutlineColor(), npc_sw_cmp.sprite.getPointsPerSegment() );
      }
    }
  }
}

void RenderGameSystem::render_background_water( sf::FloatRect player_position )
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

void RenderGameSystem::render_wormhole_effect( Sprites::Containers::TileMap &floormap )
{
  auto wormhole_view = getReg().view<Cmp::WormholeMultiBlock, Cmp::Position, Cmp::SpriteAnimation>();
  for ( auto [entity, wormhole_cmp, pos_cmp, anim_cmp] : wormhole_view.each() )
  {
    try
    {
      auto &wormhole_sprite = m_sprite_factory.get_multisprite_by_type( anim_cmp.m_sprite_type );
      // Setup shader
      m_wormhole_shader.update_shader_position( pos_cmp.position, Sprites::ViewFragmentShader::Align::TOPLEFT );

      // Draw background onto shader texture
      floormap.draw( m_wormhole_shader.get_render_texture(), sf::RenderStates::Default );

      safe_render_sprite_to_target( m_wormhole_shader.get_render_texture(), wormhole_sprite.get_sprite_type(), wormhole_cmp,
                                    anim_cmp.m_current_frame );

      // Update and draw shader
      Sprites::UniformBuilder builder;
      builder.set( "time", m_wormhole_shader.getElapsedTime().asSeconds() )
          .set( "screenSize", m_wormhole_shader.get_view_size() )
          .set( "centerPosition", m_wormhole_shader.get_view_center() );
      m_wormhole_shader.Sprites::BaseFragmentShader::update( builder );

      m_wormhole_shader.draw( m_window, sf::RenderStates::Default );
    } catch ( const std::out_of_range &e )
    {
      SPDLOG_WARN( "Missing wormhole sprite '{}' in map, rendering fallback square", "WORMHOLE" );
      render_fallback_square( pos_cmp, sf::Color::Magenta );
    }
  }
}

void RenderGameSystem::render_arrow_compass()
{
  auto player_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();

  auto [found_entt, found_carryitem_type] = Utils::get_player_inventory_type( getReg() );
  if ( not found_carryitem_type.contains( "exitkey" ) and not found_carryitem_type.contains( "cryptkey" ) and
       not found_carryitem_type.contains( "CARRYITEM.relic" ) )
    return;

  // if holding an exitkey then target the exit pos
  Cmp::Position arrow_target( { 0.f, 0.f }, { 0.f, 0.f } );
  if ( found_carryitem_type.contains( "exitkey" ) )
  {
    auto exit_view = getReg().view<Cmp::Exit, Cmp::Position>();
    for ( auto [exit_entity, exit_cmp, exit_pos_cmp] : exit_view.each() )
    {
      arrow_target = exit_pos_cmp;
    }
  }

  // if holding a cryptkey then target the nearest inactive crypt
  if ( found_carryitem_type.contains( "cryptkey" ) )
  {
    using CryptDistanceQueue = std::priority_queue<std::pair<float, Cmp::Position>, std::vector<std::pair<float, Cmp::Position>>,
                                                   Utils::Maths::DistancePositionComparator>;
    CryptDistanceQueue distance_queue;
    auto crypt_view = getReg().view<Cmp::CryptEntrance, Cmp::Position>();
    for ( auto [crypt_entity, crypt_cmp, crypt_pos_cmp] : crypt_view.each() )
    {
      if ( crypt_cmp.is_open() ) continue;
      auto float_distance = Utils::Maths::getEuclideanDistance( crypt_pos_cmp.position, Utils::get_player_position( getReg() ).position );
      distance_queue.emplace( float_distance, crypt_pos_cmp );
    }
    if ( distance_queue.empty() ) return; // there are no suitable crypts so give up
    arrow_target = distance_queue.top().second;
  }

  // if holding a relic then target the nearest inactive altar
  if ( found_carryitem_type.contains( "CARRYITEM.relic" ) )
  {
    SPDLOG_INFO( "Player is holding a relic" );
    using AltarDistanceQueue = std::priority_queue<std::pair<float, Cmp::Position>, std::vector<std::pair<float, Cmp::Position>>,
                                                   Utils::Maths::DistancePositionComparator>;
    AltarDistanceQueue distance_queue;
    auto crypt_view = getReg().view<Cmp::AltarMultiBlock>();
    for ( auto [altar_entity, altar_cmp] : crypt_view.each() )
    {
      if ( altar_cmp.are_powers_active() ) continue;
      auto float_distance = Utils::Maths::getEuclideanDistance( altar_cmp.position, Utils::get_player_position( getReg() ).position );
      distance_queue.emplace( float_distance, Cmp::Position( altar_cmp.position, altar_cmp.size ) );
    }
    if ( distance_queue.empty() ) return; // there are no suitable crypts so give up
    arrow_target = distance_queue.top().second;
  }

  for ( auto [player_entity, pc_cmp, pc_pos_cmp] : player_view.each() )
  {

    // dont show the compass arrow pointing to the exit if the exit is on-screen....we can see it
    if ( Utils::is_visible_in_view( getGameView(), arrow_target ) ) return;

    auto player_pos_center = pc_pos_cmp.getCenter();
    sf::Vector2f exit_pos_center = arrow_target.getCenter();
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
    sf::FloatRect arrow_rect{ arrow_position - sf::Vector2f{ Constants::kGridSquareSizePixelsF.x / 2.0f, Constants::kGridSquareSizePixelsF.y / 2.0f },
                              Constants::kGridSquareSizePixelsF };

    // Map sin(time) from [-1, 1] to [0.2, 1.0]
    // Formula: min + (max - min) * (sin(freq * time) + 1) / 2
    auto time = m_compass_osc_clock.getElapsedTime().asSeconds();
    auto sine = std::sin( m_compass_freq * time );
    float oscillating_scale = m_compass_min_scale + ( m_compass_max_scale - m_compass_min_scale ) * ( sine + 1.0f ) / 2.0f;
    auto scale = sf::Vector2f{ oscillating_scale, oscillating_scale };

    auto sprite_index = 0;
    auto alpha = 255;
    auto origin = sf::Vector2f{ Constants::kGridSquareSizePixelsF.x / 2.0f, Constants::kGridSquareSizePixelsF.y / 2.0f };

    safe_render_sprite( "ARROW", arrow_rect, sprite_index, scale, alpha, origin, angle_radians );
  }
}

void RenderGameSystem::render_dark_mode_shader()
{
  // Update dark mode shader with proper parameters
  auto shader_local_position = m_local_view.getCenter() - m_local_view.getSize() * 0.5f;
  sf::Vector2f aperture_half_size( Constants::kGridSquareSizePixelsF * 4.f );
  auto display_res = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::DisplayResolution>( getReg() );
  m_dark_mode_shader.update( shader_local_position, aperture_half_size, kLocalMapViewSize, display_res );
  m_window.draw( m_dark_mode_shader );
}

void RenderGameSystem::render_scryingball_doglegs()
{
  auto draw_dogleg = [this]( sf::Vector2f source_pos, sf::Vector2f target_pos, sf::Color color, float thickness )
  {
    sf::Vector2f corner{};
    if ( target_pos.y - source_pos.y < target_pos.x - source_pos.x ) { corner = sf::Vector2f{ source_pos.x, target_pos.y }; }
    else { corner = sf::Vector2f{ target_pos.x, source_pos.y }; }

    m_window.draw( Utils::Maths::thick_line_rect( source_pos, corner, color, thickness ) );
    m_window.draw( Utils::Maths::thick_line_rect( corner, target_pos, color, thickness ) );
  };

  constexpr float kLineThickness = 3.f;
  for ( auto [scryingball_ent, scryingball_cmp, scryingball_pos_cmp] : getReg().view<Cmp::ScryingBall, Cmp::Position>().each() )
  {
    if ( not scryingball_cmp.active ) continue;
    switch ( scryingball_cmp.target )
    {
      case Cmp::ScryingBall::Target::YELLOW: {
        auto altar_view = getReg().view<Cmp::AltarMultiBlock>();
        for ( auto [altar_entt, altar_cmp] : altar_view.each() )
        {
          // yellow for altar paths
          draw_dogleg( scryingball_pos_cmp.getCenter(), altar_cmp.getCenter(), sf::Color( 255, 255, 0, 128 ), kLineThickness );
        }
        break;
      }
      case Cmp::ScryingBall::Target::RED: {
        auto crypt_view = getReg().view<Cmp::CryptEntrance, Cmp::Position>();
        for ( auto [crypt_entt, crypt_cmp, crypt_pos_cmp] : crypt_view.each() )
        {
          // red for crypt paths
          draw_dogleg( scryingball_pos_cmp.getCenter(), crypt_pos_cmp.getCenter(), sf::Color( 255, 0, 0, 128 ), kLineThickness );
        }
        break;
      }
      case Cmp::ScryingBall::Target::GREEN: {
        auto exit_view = getReg().view<Cmp::Exit, Cmp::Position>();
        for ( auto [exit_entt, exit_cmp, exit_pos_cmp] : exit_view.each() )
        {
          draw_dogleg( scryingball_pos_cmp.getCenter(), exit_pos_cmp.getCenter(), sf::Color( 0, 255, 0, 128 ), kLineThickness );
        }
        break;
      }
      case Cmp::ScryingBall::Target::NONE: {
        break;
      }
    }
  }
}

} // namespace ProceduralMaze::Sys