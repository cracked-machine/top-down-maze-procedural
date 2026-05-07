
#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteOffset.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Armed.hpp>
#include <Components/Crypt/CryptChest.hpp>
#include <Components/Crypt/CryptEntrance.hpp>
#include <Components/Crypt/CryptInteriorMultiBlock.hpp>
#include <Components/Crypt/CryptLever.hpp>
#include <Components/Crypt/CryptMultiBlock.hpp>
#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomLavaPit.hpp>
#include <Components/Crypt/CryptRoomLavaPitCell.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/Exit.hpp>
#include <Components/Grave/GraveMultiBlock.hpp>
#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/Inventory/ScryingBall.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Persistent/CameraSmoothSpeed.hpp>
#include <Components/Persistent/DisplayResolution.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerBlastRadius.hpp>
#include <Components/Player/PlayerCadaverCount.hpp>
#include <Components/Player/PlayerCurse.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Player/PlayerWealth.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Ruin/RuinBuildingMultiBlock.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Components/Wormhole/WormholeMultiBlock.hpp>
#include <Components/ZOrderValue.hpp>
#include <LightningStrike.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Random.hpp>
#include <ReservedPosition.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <Shaders/BaseShaderSprite.hpp>
#include <Shaders/DarkModeShader.hpp>
#include <Shaders/DrippingBloodShader.hpp>
#include <Shaders/FloodWaterShader.hpp>
#include <Shaders/MistShader.hpp>
#include <Shaders/NightStaticShader.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/TileMap.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderOverlaySystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/ShaderSystem.hpp>
#include <Systems/Threats/HazardFieldSystemImpl.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <memory>
#include <queue>
#include <ranges>

namespace ProceduralMaze::Sys
{

RenderGameSystem::RenderGameSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                    Audio::SoundBank &sound_bank )
    : RenderSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "RenderGameSystem initialized" );
}

RenderGameSystem::~RenderGameSystem() = default;

void RenderGameSystem::render_game( sf::Time dt, RenderOverlaySystem &render_overlay_sys )
{
  using namespace Sprites;

  // check for updates to the System modes
  for ( auto [entt, sys_cmp] : reg().view<Cmp::System>().each() )
  {
    m_show_path_finding = sys_cmp.show_path_finding;
    m_show_debug_stats = sys_cmp.show_debug_stats;
    m_shaders_enabled = sys_cmp.shaders_enabled;
    m_show_playernopath = sys_cmp.show_playernopath;
    m_show_reserved = sys_cmp.show_reserved;
    m_show_npcnopath = sys_cmp.show_npcnopath;
  }

  const Cmp::Position player_pos_cmp = Utils::Player::get_position( reg() );

  // make sure the local view is centered on the player mid-point and not at their top-left corner
  // (otherwise this makes views, shaders, etc look off-center)
  update_camera( dt );

  // re-populate the z-order queue with the latest entity/component data
  refresh_z_order_queue();

  bool debug_tick = m_show_debug_stats && ( m_debug_update_timer.getElapsedTime() > m_debug_update_interval );

  // main render begin
  m_window.clear();

  // render anything with a ZOrderValue component in lowest value first order
  for ( const auto &zorder_entry : m_zorder_queue_ )
  {
    auto entity = zorder_entry.e;
    if ( reg().all_of<Cmp::Position, Cmp::SpriteAnimation>( entity ) )
    {
      const auto &pos_cmp = reg().get<Cmp::Position>( entity );
      const auto &anim_cmp = reg().get<Cmp::SpriteAnimation>( entity );

      uint8_t alpha_value = 255;
      auto *obst_cmp = reg().try_get<Cmp::AbsoluteAlpha>( entity );
      if ( obst_cmp ) alpha_value = obst_cmp->getAlpha();

      sf::Vector2f new_origin_value = { 0.F, 0.F };
      auto *new_offset_cmp = reg().try_get<Cmp::AbsoluteOffset>( entity );
      if ( new_offset_cmp ) new_origin_value = new_offset_cmp->getOffset();

      sf::Angle new_angle_value = sf::degrees( 0.f );
      auto *new_angle_cmp = reg().try_get<Cmp::AbsoluteRotation>( entity );
      if ( new_angle_cmp ) new_angle_value = sf::degrees( new_angle_cmp->getAngle() );

      safe_render_sprite_world( anim_cmp.m_sprite_type, pos_cmp, anim_cmp.getFrameIndexOffset() + anim_cmp.m_current_frame, { 1.f, 1.f }, alpha_value,
                                new_origin_value, new_angle_value );

      if ( reg().any_of<Cmp::SeeingStone>( entity ) )
      {
        const auto &stone_cmp = reg().get<Cmp::SeeingStone>( entity );
        render_seeingstone_doglegs( stone_cmp, pos_cmp );
      }

      if ( reg().any_of<Cmp::InventoryWearLevel>( entity ) )
      {
        render_overlay_sys.render_wear_level( reg().get<Cmp::InventoryWearLevel>( entity ).m_level, pos_cmp );
      }
    }
    else if ( reg().all_of<ParticleSpriteOwner>( entity ) )
    {
      auto &particle_sprite_owner = reg().get<ParticleSpriteOwner>( entity );
      // pass the world view so the sprite can map world coords to screen coords
      particle_sprite_owner.sprite->set_view_transform( m_window, s_world_view );
      draw_screen( *particle_sprite_owner.sprite );
    }
    else if ( reg().all_of<ShaderSpriteOwner>( entity ) )
    {
      auto &shader_sprite_owner = reg().get<ShaderSpriteOwner>( entity );
      if ( not shader_sprite_owner.sprite ) continue;
      shader_sprite_owner.sprite->update( reg() );
      if ( m_shaders_enabled ) { draw_world( *shader_sprite_owner.sprite ); }
    }
    else if ( reg().all_of<Sprites::Containers::TileMap>( entity ) )
    {

      auto &floor_tiles = reg().get<Sprites::Containers::TileMap>( entity );
      sf::Vector2f adjusted{ static_cast<float>( floor_tiles.world_grid_offset.x ) * Constants::kGridSizePxF.x,
                             static_cast<float>( floor_tiles.world_grid_offset.y ) * Constants::kGridSizePxF.y };
      floor_tiles.setPosition( adjusted );
      draw_world( floor_tiles );
    }
  }

  // finally render anything on top
  render_armed();
  render_shockwaves();
  render_arrow_compass();

  // lava pit outline
  render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomLavaPit>( sf::Color( 64, 64, 64 ), 0.5f );

  render_lightning_strike();
  render_overlay_sys.render_shop_inventory_overlay();

  // debug: show crypt component boundaries
  if ( m_show_debug_stats )
  {

    render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomLavaPitCell>( sf::Color( 254, 128, 32 ), 0.5f );
    render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomOpen>( sf::Color::Green, 1.f );
    render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomStart>( sf::Color::Blue, 1.f );
    render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomEnd>( sf::Color::Yellow, 1.f );
    render_overlay_sys.render_square_for_floatrect_cmp<Cmp::CryptRoomClosed>( sf::Color::Red, 1.f );
    render_overlay_sys.render_square_for_vector2f_cmp<Cmp::CryptPassageBlock>( sf::Color::Black, 1.f );
  }
  if ( m_show_npcnopath )
  {
    for ( auto [entt, npcnopath_cmp, pos_cmp] : reg().view<Cmp::NpcNoPathFinding, Cmp::Position>().each() )
    {
      auto rectbounds = Cmp::RectBounds::scaled( pos_cmp.position, pos_cmp.size, 1.f );
      render_rectbounds( rectbounds, sf::Color::Red );
    }
  }

  if ( m_show_npcnopath )
  {
    for ( auto [entt, npcnopath_cmp, pos_cmp] : reg().view<Cmp::NpcNoPathFinding, Cmp::Position>().each() )
    {
      auto rectbounds = Cmp::RectBounds::scaled( pos_cmp.position, pos_cmp.size, 1.f );
      render_rectbounds( rectbounds, sf::Color::Red );
    }
  }
  if ( m_show_playernopath )
  {
    for ( auto [entt, playernopath_cmp, pos_cmp] : reg().view<Cmp::PlayerNoPath, Cmp::Position>().each() )
    {
      auto rectbounds = Cmp::RectBounds::scaled( pos_cmp.position, pos_cmp.size, 1.f );
      render_rectbounds( rectbounds, sf::Color::Red );
    }
  }
  if ( m_show_reserved )
  {
    for ( auto [entt, reserved_cmp, pos_cmp] : reg().view<Cmp::ReservedPosition, Cmp::Position>().each() )
    {
      auto rectbounds = Cmp::RectBounds::scaled( pos_cmp.position, pos_cmp.size, 1.f );
      render_rectbounds( rectbounds, sf::Color::Red );
    }
  }

  if ( m_show_path_finding )
  {
    render_overlay_sys.render_lerp_positions();
    render_overlay_sys.render_spatial_grid_neighbours( player_pos_cmp, sf::Color::Cyan, PathFinding::QueryCompass::CARDINAL );

    for ( auto [npc_entt, npc_cmp, npc_pos_cmp, anim_cmp] : reg().view<Cmp::NPC, Cmp::Position, Cmp::SpriteAnimation>().each() )
    {
      auto query_compass = PathFinding::QueryCompass::CARDINAL;
      if ( anim_cmp.m_sprite_type.contains( "sprite.ghost" ) ) query_compass = PathFinding::QueryCompass::BOTH;
      render_overlay_sys.render_spatial_grid_neighbours( npc_pos_cmp, sf::Color::Magenta, query_compass );
      render_overlay_sys.render_pathfinding_vector( npc_pos_cmp, player_pos_cmp, sf::Color::White, query_compass );
    }
  }

  // float start_y_pos = 0;
  render_overlay_sys.render_ui_outlines();
  render_overlay_sys.render_ui_icons();
  render_overlay_sys.render_ui_inventory_icon();
  render_overlay_sys.render_ui_meters();
  render_overlay_sys.render_ui_labels( dt );
  render_overlay_sys.render_ui_texts();
  render_overlay_sys.render_level_depth();

  auto display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg() );
  render_overlay_sys.render_crypt_maze_timer( { static_cast<float>( display_size.x ) / 2.f, 0.f }, 100 );

  if ( m_show_debug_stats )
  {
    if ( debug_tick )
    {
      render_overlay_sys.render_ui_player_position();
      render_overlay_sys.render_ui_mouse_position();
      render_overlay_sys.render_ui_stats();
      render_overlay_sys.render_ui_zorder_list( m_zorder_queue_ );
      render_overlay_sys.render_ui_npc_list();

      m_debug_update_timer.restart();
    }
  }

  // restart once after all debug blocks
  if ( debug_tick ) { m_debug_update_timer.restart(); }

  m_window.display();
}

void RenderGameSystem::refresh_z_order_queue()
{
  m_zorder_queue_.clear();
  sf::FloatRect view_bounds = Utils::calculate_view_bounds( s_world_view );

  // prevent pop-in/pop-outs when multiblock entities are near the edge of the view
  add_visible_entity_to_z_order_queue<Cmp::AltarMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::CryptMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::GraveMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::HolyWellMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::CryptInteriorMultiBlock>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<Cmp::RuinBuildingMultiBlock>( m_zorder_queue_, view_bounds );

  // add any floor tile sets
  add_visible_entity_to_z_order_queue<Sprites::Containers::TileMap>( m_zorder_queue_, view_bounds );

  // add the wrapper types for all particle and shader sprites so they can be rendered with the other entities
  add_visible_entity_to_z_order_queue<ParticleSpriteOwner>( m_zorder_queue_, view_bounds );
  add_visible_entity_to_z_order_queue<ShaderSpriteOwner>( m_zorder_queue_, view_bounds );

  // add other components as normal
  add_visible_entity_to_z_order_queue<Cmp::Position>( m_zorder_queue_, view_bounds );

  std::ranges::sort( m_zorder_queue_, []( const ZOrder &a, const ZOrder &b ) { return a.z < b.z; } );
}

void RenderGameSystem::init_world_view()
{
  // init world view dimensions
  s_world_view = sf::View( { kWorldViewSizeF.x * 0.5f, kWorldViewSizeF.y * 0.5f }, kWorldViewSizeF );
  s_world_view.setViewport( sf::FloatRect( { 0.f, 0.f }, { 1.f, 1.f } ) );

  auto start_pos = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( reg() );
  s_world_view.setCenter( start_pos );
}

void RenderGameSystem::update_camera( sf::Time deltaTime )
{

  // Get the player's current position
  auto player_view = reg().view<Cmp::PlayerCharacter, Cmp::Position>();
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
    auto camera_smooth_speed = Sys::PersistSystem::get<Cmp::Persist::CameraSmoothSpeed>( reg() ).get_value();
    float t = 1.0f - std::exp( -camera_smooth_speed * dt ); // Exponential smoothing

    m_camera_position.x += ( target_position.x - m_camera_position.x ) * t;
    m_camera_position.y += ( target_position.y - m_camera_position.y ) * t;

    // Snap to target if very close (prevents endless micro-adjustments)
    constexpr float kSnapThreshold = 0.1f;
    if ( std::abs( target_position.x - m_camera_position.x ) < kSnapThreshold &&
         std::abs( target_position.y - m_camera_position.y ) < kSnapThreshold )
    {
      m_camera_position = target_position;
    }

    // Update the view center
    sf::Vector2f view_center = m_camera_position + ( pos_cmp.size / 2.f );
    s_world_view.setCenter( view_center );
  }
}

void RenderGameSystem::render_armed()
{
  // render armed obstacles with debug outlines
  auto armed_view = reg().view<Cmp::Armed, Cmp::Position>();
  for ( auto [entity, armed_cmp, pos_cmp] : armed_view.each() )
  {
    if ( armed_cmp.m_display_bomb_sprite ) { safe_render_sprite_world( "sprite.item.bomb", pos_cmp, 0 ); }

    sf::RectangleShape temp_square( Constants::kGridSizePxF );
    temp_square.setPosition( pos_cmp.position );
    temp_square.setOutlineColor( sf::Color::Transparent );
    temp_square.setFillColor( sf::Color::Transparent );
    if ( armed_cmp.getElapsedWarningTime() > armed_cmp.m_warning_delay )
    {
      temp_square.setOutlineColor( armed_cmp.m_armed_color );
      temp_square.setFillColor( armed_cmp.m_armed_color );
    }
    temp_square.setOutlineThickness( 1.f );
    draw_world( temp_square );
  }
}

void RenderGameSystem::render_shockwaves()
{
  for ( auto [npc_sh_entt, npc_sw_cmp] : reg().view<Cmp::NpcShockwave>().each() )
  {
    for ( const auto &segment : npc_sw_cmp.sprite.getVisibleSegments() )
    {
      sf::FloatRect segment_bounds = segment.getBounds( npc_sw_cmp.sprite.getPosition(), npc_sw_cmp.sprite.getRadius(),
                                                        npc_sw_cmp.sprite.getOutlineThickness() );

      if ( Utils::is_visible_in_view( RenderSystem::get_world_view(), segment_bounds ) )
      {
        segment.draw( m_window, sf::RenderStates::Default, npc_sw_cmp.sprite.getPosition(), npc_sw_cmp.sprite.getRadius(),
                      npc_sw_cmp.sprite.getOutlineThickness(), npc_sw_cmp.sprite.getOutlineColor(), npc_sw_cmp.sprite.getPointsPerSegment() );
      }
    }
  }
}

void RenderGameSystem::render_arrow_compass()
{
  auto player_view = reg().view<Cmp::PlayerCharacter, Cmp::Position>();

  auto [found_entt, found_carryitem_type] = Utils::Player::get_inventory_type( reg() );

  if ( not found_carryitem_type.contains( "exitkey" ) and not found_carryitem_type.contains( "cryptkey" ) and
       not found_carryitem_type.contains( "relic" ) )
    return;

  // if holding an exitkey then target the exit pos
  Cmp::Position arrow_target( { 0.f, 0.f }, { 0.f, 0.f } );
  if ( found_carryitem_type.contains( "exitkey" ) )
  {
    auto exit_view = reg().view<Cmp::Exit, Cmp::Position>();
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
    auto crypt_view = reg().view<Cmp::CryptEntrance, Cmp::Position>();
    for ( auto [crypt_entity, crypt_cmp, crypt_pos_cmp] : crypt_view.each() )
    {
      if ( crypt_cmp.is_open() ) continue;
      auto float_distance = Utils::Maths::getEuclideanDistance( crypt_pos_cmp.position, Utils::Player::get_position( reg() ).position );
      distance_queue.emplace( float_distance, crypt_pos_cmp );
    }
    if ( distance_queue.empty() ) return; // there are no suitable crypts so give up
    arrow_target = distance_queue.top().second;
  }

  // if holding a relic then target the nearest inactive altar
  if ( found_carryitem_type.contains( "relic" ) )
  {
    using AltarDistanceQueue = std::priority_queue<std::pair<float, Cmp::Position>, std::vector<std::pair<float, Cmp::Position>>,
                                                   Utils::Maths::DistancePositionComparator>;
    AltarDistanceQueue distance_queue;
    auto crypt_view = reg().view<Cmp::AltarMultiBlock>();
    for ( auto [altar_entity, altar_cmp] : crypt_view.each() )
    {
      if ( altar_cmp.is_exitkey_lockout() ) continue;
      auto float_distance = Utils::Maths::getEuclideanDistance( altar_cmp.position, Utils::Player::get_position( reg() ).position );
      distance_queue.emplace( float_distance, Cmp::Position( altar_cmp.position, altar_cmp.size ) );
    }
    if ( distance_queue.empty() ) return; // there are no suitable crypts so give up
    arrow_target = distance_queue.top().second;
  }

  for ( auto [player_entity, pc_cmp, pc_pos_cmp] : player_view.each() )
  {

    // dont show the compass arrow pointing to the exit if the exit is on-screen....we can see it
    if ( Utils::is_visible_in_view( get_world_view(), arrow_target ) ) return;

    auto player_pos_center = pc_pos_cmp.getCenter();
    sf::Vector2f exit_pos_center = arrow_target.getCenter();
    sf::Vector2f direction = ( exit_pos_center - player_pos_center ).normalized();

    // Get view bounds in world coordinates
    sf::Vector2f view_center = s_world_view.getCenter();
    sf::Vector2f view_size = s_world_view.getSize();
    sf::FloatRect view_bounds{ { view_center.x - ( view_size.x / 2.0f ), view_center.y - ( view_size.y / 2.0f ) }, view_size };

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
    sf::FloatRect arrow_rect{ arrow_position - sf::Vector2f{ Constants::kGridSizePxF.x / 2.0f, Constants::kGridSizePxF.y / 2.0f },
                              Constants::kGridSizePxF };

    // Map sin(time) from [-1, 1] to [0.2, 1.0]
    // Formula: min + (max - min) * (sin(freq * time) + 1) / 2
    auto time = m_compass_osc_clock.getElapsedTime().asSeconds();
    auto sine = std::sin( m_compass_freq * time );
    float oscillating_scale = m_compass_min_scale + ( ( m_compass_max_scale - m_compass_min_scale ) * ( sine + 1.0f ) / 2.0f );
    auto scale = sf::Vector2f{ oscillating_scale, oscillating_scale };

    auto sprite_index = 0;
    auto alpha = 255;
    auto origin = sf::Vector2f{ Constants::kGridSizePxF.x / 2.0f, Constants::kGridSizePxF.y / 2.0f };

    safe_render_sprite_world( "sprite.graveyard.arrow", arrow_rect, sprite_index, scale, alpha, origin, angle_radians );
  }
}

void RenderGameSystem::render_seeingstone_doglegs( const Cmp::SeeingStone &stone_cmp, const Cmp::Position &pos_cmp )
{
  auto draw_dogleg = [this]( sf::Vector2f source_pos, sf::Vector2f target_pos, sf::Color color, float thickness )
  {
    sf::Vector2f corner{};
    if ( target_pos.y - source_pos.y < target_pos.x - source_pos.x ) { corner = sf::Vector2f{ source_pos.x, target_pos.y }; }
    else { corner = sf::Vector2f{ target_pos.x, source_pos.y }; }

    draw_world( Utils::Maths::thick_line_rect( source_pos, corner, color, thickness ) );
    draw_world( Utils::Maths::thick_line_rect( corner, target_pos, color, thickness ) );
  };

  constexpr float kLineThickness = 3.f;
  // for ( auto [seeingstone_ent, stone_cmp, pos_cmp] : reg().view<Cmp::SeeingStone, Cmp::Position>().each() )
  {
    if ( not stone_cmp.active ) { return; }
    switch ( stone_cmp.target )
    {
      case Cmp::SeeingStone::Target::YELLOW: {
        auto altar_view = reg().view<Cmp::AltarMultiBlock>();
        for ( auto [altar_entt, altar_cmp] : altar_view.each() )
        {
          // yellow for altar paths
          draw_dogleg( pos_cmp.getCenter(), altar_cmp.getCenter(), sf::Color( 255, 255, 0, 128 ), kLineThickness );
        }
        break;
      }
      case Cmp::SeeingStone::Target::RED: {
        auto crypt_view = reg().view<Cmp::CryptEntrance, Cmp::Position>();
        for ( auto [crypt_entt, crypt_cmp, crypt_pos_cmp] : crypt_view.each() )
        {
          // red for crypt paths
          draw_dogleg( pos_cmp.getCenter(), crypt_pos_cmp.getCenter(), sf::Color( 255, 0, 0, 128 ), kLineThickness );
        }
        break;
      }
      case Cmp::SeeingStone::Target::GREEN: {
        auto exit_view = reg().view<Cmp::Exit, Cmp::Position>();
        for ( auto [exit_entt, exit_cmp, exit_pos_cmp] : exit_view.each() )
        {
          draw_dogleg( pos_cmp.getCenter(), exit_pos_cmp.getCenter(), sf::Color( 0, 255, 0, 128 ), kLineThickness );
        }
        break;
      }
      case Cmp::SeeingStone::Target::NONE: {
        break;
      }
    }
  }
}

void RenderGameSystem::render_lightning_strike()
{

  const auto kAuxStrikeLineColor = sf::Color( 255, 255, 255, 255 );
  const auto kMainStrikeLineColor = sf::Color( 0, 255, 255, 255 );

  const float kMainLineThickness = 10.f;
  const float kAuxLineThickness = 3.f;

  // Get the first LightningStrike only. Once it expires the next LightningStrike will be at the front.
  auto view = reg().view<Cmp::LightningStrike>();
  if ( view.size() == 0 ) return;
  auto &cmp = reg().get<Cmp::LightningStrike>( view.front() );
  cmp.timer.start();

  if ( cmp.sequence.size() < 2 )
  {
    SPDLOG_WARN( "Lightning component has empty sequence. Skipping rendering step." );
    return;
  }

  render_screen_flash( sf::Color( 255, 255, 255, 180 ) );

  // Draw the sequence of vertices by iterating pairs of vertices from the current and next row.
  // Main strike line is index zero (thick/blue). Aux strike lines are other indices (thin/white).
  const auto &ls_seq_row = cmp.sequence;
  for ( auto curr_row_iter = ls_seq_row.begin(); curr_row_iter < ls_seq_row.end(); curr_row_iter++ )
  {
    auto next_row_iter = std::next( curr_row_iter );
    if ( next_row_iter == ls_seq_row.end() ) { break; }

    // next row's convergence point - all vertices in the current row connect to this
    sf::Vector2f converge_pos = world_to_screen( next_row_iter->at( 0 ).position );

    for ( auto [curr_row_idx, current_vertex] : std::views::enumerate( *curr_row_iter ) )
    {
      sf::Vector2f first_pos = world_to_screen( current_vertex.position );

      // always converge non-zero index vertex back to the main line (zero-index)
      if ( curr_row_idx > 0 ) { draw_screen( Utils::Maths::thick_line_rect( first_pos, converge_pos, kAuxStrikeLineColor, kAuxLineThickness ) ); }
      else if ( curr_row_idx == 0 )
      {
        // always draw main line on zero-index
        draw_screen( Utils::Maths::thick_line_rect( first_pos, converge_pos, kMainStrikeLineColor, kMainLineThickness ) );

        for ( auto [next_row_idx, next_vertex] : std::views::enumerate( *next_row_iter ) )
        {
          // always diverge zero-index vertex out to available non-zero index vertex on next row
          if ( next_row_idx > 0 )
          {
            sf::Vector2f diverge_pos = world_to_screen( next_vertex.position );
            draw_screen( Utils::Maths::thick_line_rect( first_pos, diverge_pos, kAuxStrikeLineColor, kAuxLineThickness ) );
          }
        }
      }
    }
  }
}

void RenderGameSystem::render_screen_flash( sf::Color color )
{

  auto display_res = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg() );
  auto flash = sf::RectangleShape( sf::Vector2f( display_res ) );
  flash.setPosition( { 0.f, 0.f } );
  flash.setFillColor( color );
  // draw flash in screen view so it covers the whole screen in screen-space
  draw_screen( flash );
}

} // namespace ProceduralMaze::Sys