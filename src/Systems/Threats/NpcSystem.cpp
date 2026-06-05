#include <Audio/SoundBank.hpp>
#include <Collision.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/AnimData.hpp>
#include <Components/Crypt/CryptSegment.hpp>
#include <Components/Direction.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcContainer.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/NpcActivateScale.hpp>
#include <Components/Persistent/NpcShockwaveMaxRadius.hpp>
#include <Components/Persistent/NpcShockwaveSpeed.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/System.hpp>
#include <Components/UUID.hpp>
#include <Components/Wall.hpp>
#include <Components/Wormhole/WormholeJump.hpp>
#include <Components/ZOrderValue.hpp>
#include <Crypt/CryptObjectiveSegment.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ParticleFactory.hpp>
#include <Factory/SpriteFactory.hpp>
#include <Grave/GraveSegment.hpp>
#include <Npc/NpcFriendly.hpp>
#include <Npc/NpcLerpSpeed.hpp>
#include <Npc/NpcTarget.hpp>
#include <PathFinding/AStar.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Persistent/DisplayResolution.hpp>
#include <Position.hpp>
#include <Random.hpp>
#include <Ruin/RuinSegment.hpp>
#include <SFML/Audio/Sound.hpp>
#include <Stats/BaseAction.hpp>
#include <Stats/CollisionAction.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/ShockwaveSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Npc.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Time.hpp>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <typeindex>

namespace Game::Sys
{

NpcSystem::NpcSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "NpcSystem initialized" );
  m_wisp_target_reset_clock.reset();
}

void NpcSystem::update( sf::Time dt )
{

  // run skelton activation checks at 5Hz
  static constexpr float kBonesInterval = 0.20f;
  m_bones_accumulator += dt;
  if ( m_bones_accumulator.asSeconds() >= kBonesInterval )
  {
    check_bones_reanimation();
    m_bones_accumulator = sf::Time::Zero;
  }

  update_pathfinding( dt );

  // run animation checks at 20Hz
  static constexpr float kAnimationInterval = 0.05f;
  m_animation_accumulator += dt;
  if ( m_animation_accumulator.asSeconds() >= kAnimationInterval )
  {
    update_animation();
    m_animation_accumulator = sf::Time::Zero;
  }
  update_sfx();
  update_movement( dt );

  if ( not Utils::getSystemCmp( reg() ).collisions_disabled )
  {
    check_once_collision();
    check_timed_collision( dt );
  }

  update_shockwaves( dt );
}

void NpcSystem::check_bones_reanimation()
{

  auto player_pos = Utils::Player::get_position( reg() );
  auto npccontainer_collision_view = reg().view<Cmp::NpcContainer, Cmp::Position>();

  for ( auto [npccontainer_entt, npccontainer_cmp, npccontainer_pos_cmp] : npccontainer_collision_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::get_world_view(), npccontainer_pos_cmp ) ) continue;

    auto &npc_activate_scale = Sys::PersistSystem::get<Cmp::Persist::NpcActivateScale>( reg() );
    // we just create a temporary RectBounds here instead of a component because we only need it
    // for this one comparison and it already contains the needed scaling logic
    auto npc_activate_bounds = Cmp::RectBounds::scaled( npccontainer_pos_cmp.position, Constants::kGridSizePxF, npc_activate_scale.get_value() );

    if ( player_pos.findIntersection( npc_activate_bounds.getBounds() ) ) { Factory::create_npc( reg(), npccontainer_entt, "npc.skeleton" ); }
  }
}

void NpcSystem::spawn_wisp()
{
  // allow only max number of wisp
  const static int MAX_WISP_COUNT = 1;
  int wisp_count = 0;
  for ( auto [npc_entt, npc_cmp] : reg().view<Cmp::NPC>().each() )
  {
    if ( npc_cmp.sprite_type_list.front().contains( "wisp" ) ) wisp_count++;
  }
  if ( wisp_count >= MAX_WISP_COUNT ) return;

  // find a random start position in the game area and create a new npc at that position
  auto [spawn_entt, spawn_pos] = Utils::Rnd::get_random_position( reg(), {}, {} );
  auto npc_entt = Factory::create_npc( reg(), spawn_entt, "npc.wisp" );
  SPDLOG_INFO( "Created wisp npc {}", static_cast<uint32_t>( npc_entt ) );

  // find a random target position in the game area that is distant from the spawn_pos
  static constexpr float kMinTargetDistance = 100.f;
  auto [target_entt, target_pos] = Utils::Rnd::get_random_position( reg(), {}, {} );
  while ( Utils::Maths::getEuclideanDistance( target_pos.position, spawn_pos.position ) < kMinTargetDistance )
  {
    auto [new_target_entt, new_target_pos] = Utils::Rnd::get_random_position( reg(), {}, {} );
    target_entt = new_target_entt;
    target_pos = new_target_pos;
  }
  reg().emplace_or_replace<Cmp::NpcTarget>( target_entt, npc_entt );

  // register the wisp in the open navmesh so A* can find a valid start cell
  if ( auto open_navmesh = m_open_navmesh.lock() )
  {
    open_navmesh->insert( npc_entt, spawn_pos );
    open_navmesh->insert( target_entt, target_pos );
  }

  SPDLOG_INFO( "Spawned wisp {} at {},{}. Target is {},{}", static_cast<uint32_t>( npc_entt ), spawn_pos.x(), spawn_pos.y(), target_pos.x(),
               target_pos.y() );
}

void NpcSystem::reset_wisp_target( entt::entity wisp_entt )
{

  const static float kWispTargetResetTimeout = 10.f;
  if ( m_wisp_target_reset_clock.getElapsedTime().asSeconds() < kWispTargetResetTimeout ) return;

  for ( auto [old_target_entt, old_npc_target_cmp, old_npc_target_pos_cmp] : reg().view<Cmp::NpcTarget, Cmp::Position>().each() )
  {
    if ( old_npc_target_cmp.id != wisp_entt ) continue;

    // find a random target position in the game area that is distant from the spawn_pos
    static constexpr float kMinTargetDistance = 100.f;
    auto [new_target_entt, new_target_pos] = Utils::Rnd::get_random_position( reg(), {}, {} );
    while ( Utils::Maths::getEuclideanDistance( new_target_pos.position, old_npc_target_pos_cmp.position ) < kMinTargetDistance )
    {
      auto [retry_target_entt, retry_target_pos] = Utils::Rnd::get_random_position( reg(), {}, {} );
      new_target_entt = retry_target_entt;
      new_target_pos = retry_target_pos;
    }
    reg().remove<Cmp::NpcTarget>( old_target_entt ); // remove from old target first
    reg().emplace_or_replace<Cmp::NpcTarget>( new_target_entt, wisp_entt );

    SPDLOG_INFO( "Reset wisp {} target to {},{}", static_cast<uint32_t>( wisp_entt ), new_target_pos.x(), new_target_pos.y() );
    m_wisp_target_reset_clock.reset();
    break; // only one target per wisp
  }
}

void NpcSystem::update_animation()
{
  for ( auto [npc_entt, npc_cmp, npc_dir_cmp, anim_cmp] : reg().view<Cmp::NPC, Cmp::Direction, Cmp::AnimData>().each() )
  {

    if ( npc_dir_cmp == sf::Vector2f( 0.0f, 0.0f ) )
    {
      anim_cmp.m_enabled = false;
      continue;
    }

    anim_cmp.m_enabled = true;
    if ( anim_cmp.m_sprite_type.contains( "sprite.skeleton" ) )
    {

      if ( npc_dir_cmp.x > 0 ) { anim_cmp.m_sprite_type = "sprite.skeleton.walk.east"; }
      else if ( npc_dir_cmp.x < 0 ) { anim_cmp.m_sprite_type = "sprite.skeleton.walk.west"; }
      else if ( npc_dir_cmp.y < 0 ) { anim_cmp.m_sprite_type = "sprite.skeleton.walk.north"; }
      else if ( npc_dir_cmp.y > 0 ) { anim_cmp.m_sprite_type = "sprite.skeleton.walk.south"; }
    }
    else if ( anim_cmp.m_sprite_type.contains( "sprite.ghost" ) )
    {
      // Ghost NPCs face cardinal directions only
      if ( npc_dir_cmp.x > 0 ) { anim_cmp.m_sprite_type = "sprite.ghost.walk.east"; }
      else if ( npc_dir_cmp.x < 0 ) { anim_cmp.m_sprite_type = "sprite.ghost.walk.west"; }
      else if ( npc_dir_cmp.y < 0 ) { anim_cmp.m_sprite_type = "sprite.ghost.walk.north"; }
      else if ( npc_dir_cmp.y > 0 ) { anim_cmp.m_sprite_type = "sprite.ghost.walk.south"; }
    }
    else if ( anim_cmp.m_sprite_type.contains( "sprite.wisp" ) )
    {
      if ( npc_dir_cmp.x > 0 ) { anim_cmp.m_sprite_type = "sprite.wisp.east"; }
      else if ( npc_dir_cmp.x < 0 ) { anim_cmp.m_sprite_type = "sprite.wisp.west"; }
    }
  }
}

void NpcSystem::update_sfx()
{
  bool any_skeleton_moving = false;

  for ( auto [npc_entt, npc_cmp, npc_dir_cmp, anim_cmp] : reg().view<Cmp::NPC, Cmp::Direction, Cmp::AnimData>().each() )
  {
    if ( anim_cmp.m_sprite_type.contains( "sprite.skeleton" ) )
    {
      if ( npc_dir_cmp != sf::Vector2f{ 0.0f, 0.0f } )
      {
        any_skeleton_moving = true;
        break;
      }
    }
  }

  auto &sfx = m_sound_bank.get_effect( "skeleton_moving" );
  if ( any_skeleton_moving )
  {
    if ( sfx.getStatus() != sf::Sound::Status::Playing ) { sfx.play(); }
  }
  else { sfx.stop(); }
}

void NpcSystem::update_pathfinding( sf::Time dt )
{
  static constexpr float kScanInterval = 0.10f;
  m_scan_accumulator += dt;
  if ( m_scan_accumulator.asSeconds() >= kScanInterval )
  {
    // Wisp NPCs — driven from NpcTarget view, no inner loop needed
    if ( auto navmesh = m_open_navmesh.lock() )
    {
      for ( auto [target_entt, npc_target_cmp, npc_target_pos_cmp] : reg().view<Cmp::NpcTarget, Cmp::Position>().each() )
      {
        if ( not reg().valid( npc_target_cmp.id ) ) continue;
        update_pathfinding_for( *navmesh, npc_target_pos_cmp, npc_target_cmp.id );
      }
    }

    // Other NPCs — target is always the player
    const Cmp::Position player_pos = Utils::Player::get_position( reg() );
    if ( auto navmesh = m_pathfinding_navmesh.lock() )
    {
      for ( auto [npc_entt, npc_cmp] : reg().view<Cmp::NPC>().each() )
      {
        if ( npc_cmp.sprite_type_list.front().contains( "wisp" ) ) continue;
        update_pathfinding_for( *navmesh, player_pos, npc_entt );
      }
    }

    m_scan_accumulator = sf::Time::Zero;
  }
}
void NpcSystem::update_pathfinding_for( PathFinding::SpatialHashGrid &navmesh, const Cmp::Position &target_pos, entt::entity npc_entity )
{

  auto target_is_in_spawn = Utils::Player::is_in_spawn( reg(), target_pos );

  auto *npc_anim_cmp = reg().try_get<Cmp::AnimData>( npc_entity );
  if ( not npc_anim_cmp ) return;
  auto npc_type = npc_anim_cmp->m_sprite_type;

  auto *npc_pos_cmp = reg().try_get<Cmp::Position>( npc_entity );
  if ( not npc_pos_cmp ) return;

  // only pathfind when in the current view/screen - except wisps - they need to pathfind at all times.
  if ( not npc_type.contains( "wisp" ) and not Utils::is_visible_in_view( RenderSystem::get_world_view(), *npc_pos_cmp ) )
  {
    reg().emplace_or_replace<Cmp::Direction>( npc_entity, Cmp::Direction( { 0.0, 0.0 } ) );
    return;
  }

  // don't intterupt NPC mid-lerp or it causes indecisive pathfinding
  auto *npc_lerp_pos_cmp = reg().try_get<Cmp::LerpPosition>( npc_entity );
  if ( npc_lerp_pos_cmp && npc_lerp_pos_cmp->m_lerp_factor < 1.0f ) return;

  // allow ghosts to sneak through gaps
  auto query_compass = PathFinding::QueryCompass::CARDINAL;

  if ( npc_type.contains( "sprite.ghost" ) ) query_compass = PathFinding::QueryCompass::BOTH;

  std::vector<PathFinding::PathNode> path;
  path = PathFinding::astar( reg(), navmesh, *npc_pos_cmp, target_pos, query_compass );

  SPDLOG_DEBUG( "{} pathsize: {}", static_cast<uint32_t>( npc_entity ), path.size() );
  if ( path.size() > 1 )
  {

    Cmp::Position next_npc_pos = path[1].pos;

    // If player is in spawn, only stop when the very next step would cross into spawn.
    // This lets the NPC walk the full path to the boundary before stopping.
    if ( ( target_is_in_spawn and Utils::Player::is_in_spawn( reg(), next_npc_pos ) ) )
    {
      reg().emplace_or_replace<Cmp::Direction>( npc_entity, Cmp::Direction( { 0.0f, 0.0f } ) );
      return;
    }

    // calculate the direction and update the NPC lerp
    auto *npc_lerp_speed_cmp = reg().try_get<Cmp::NpcLerpSpeed>( npc_entity );
    if ( not npc_lerp_speed_cmp ) return;
    auto candidate_lerp_pos = Cmp::LerpPosition( next_npc_pos.position, npc_lerp_speed_cmp->speed );
    auto distance_to_target = next_npc_pos.position - npc_pos_cmp->position;
    if ( distance_to_target == sf::Vector2f( 0.0f, 0.0f ) ) return;

    // prevent NPC warping via another NPCs pathfinding
    const bool too_far = std::abs( distance_to_target.x ) >= Constants::kGridSizePxF.x * 1.5f ||
                         std::abs( distance_to_target.y ) >= Constants::kGridSizePxF.y * 1.5f;
    if ( too_far ) return;

    auto norm_direction = Cmp::Direction( distance_to_target.normalized() );

    reg().emplace_or_replace<Cmp::Direction>( npc_entity, norm_direction );
    reg().emplace_or_replace<Cmp::LerpPosition>( npc_entity, candidate_lerp_pos );
  }
  else
  {
    if ( npc_type.contains( "wisp" ) )
    {
      if ( not m_wisp_target_reset_clock.isRunning() ) { m_wisp_target_reset_clock.restart(); }
      reset_wisp_target( npc_entity );
    }
    else { reg().emplace_or_replace<Cmp::Direction>( npc_entity, Cmp::Direction( { 0.0, 0.0 } ) ); }
  }
}

void NpcSystem::update_movement( sf::Time dt )
{

  for ( auto [npc_entt, npc_cmp, npc_pos_cmp] : reg().view<Cmp::NPC, Cmp::Position>().each() )
  {
    PathFinding::SpatialHashGridSharedPtr navmesh;
    if ( npc_cmp.sprite_type_list.front().contains( "wisp" ) )
    {

      SPDLOG_DEBUG( "Update wisp position at {},{}", npc_pos_cmp.x(), npc_pos_cmp.y() );
      navmesh = m_open_navmesh.lock();
      if ( not navmesh ) continue;
    }
    else
    {
      navmesh = m_pathfinding_navmesh.lock();
      if ( not navmesh ) continue;
    }
    update_movement_for( *navmesh, npc_entt, dt );
  }
}

void NpcSystem::update_movement_for( PathFinding::SpatialHashGrid &navmesh, entt::entity npc_entity, sf::Time dt )
{

  // if there is an obstacle at this entity move onto the next entity
  auto *obst_cmp = reg().try_get<Cmp::Obstacle>( npc_entity );
  if ( obst_cmp ) return;

  // If this is the first update, store the start position
  auto *lerp_pos_cmp = reg().try_get<Cmp::LerpPosition>( npc_entity );
  auto *pos_cmp = reg().try_get<Cmp::Position>( npc_entity );
  if ( not pos_cmp or not lerp_pos_cmp ) return;
  if ( lerp_pos_cmp->m_lerp_factor == 0.0f )
  {
    // Allow NPCs to escape wormholes if they're mid-lerp.
    if ( reg().try_get<Cmp::WormholeJump>( npc_entity ) ) return;

    lerp_pos_cmp->m_start = pos_cmp->position;
  }

  lerp_pos_cmp->m_lerp_factor += lerp_pos_cmp->m_lerp_speed * dt.asSeconds();

  // lerp has completed
  if ( lerp_pos_cmp->m_lerp_factor >= 1.0f )
  {
    auto old_position = *pos_cmp;
    pos_cmp->position = lerp_pos_cmp->m_target;
    reg().remove<Cmp::LerpPosition>( npc_entity );
    navmesh.update( npc_entity, old_position, *pos_cmp );
  }
  else
  {
    // Lerp from start to target directly
    // Simple manual lerp - 33 lines of assembly vs 134 for std::lerp vs 54 for std::fma - https://godbolt.org/z/YdeKco5d6
    const float t = lerp_pos_cmp->m_lerp_factor;
    const float one_minus_t = 1.0f - t;
    pos_cmp->position.x = one_minus_t * lerp_pos_cmp->m_start.x + t * lerp_pos_cmp->m_target.x;
    pos_cmp->position.y = one_minus_t * lerp_pos_cmp->m_start.y + t * lerp_pos_cmp->m_target.y;
  }

  // add additional zorder for Wisp NPCs (read from JSON)
  float zorder_augment = 0.f;
  if ( Utils::Npc::get_sprite_type( reg(), npc_entity ).contains( "wisp" ) )
  {
    const auto &spritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.wisp.east" );
    zorder_augment = spritesheet.get_zorder( 0 );
  }
  reg().patch<Cmp::ZOrderValue>( npc_entity, [&]( auto &zorder_cmp ) { zorder_cmp.setZOrder( pos_cmp->position.y + zorder_augment ); } );
}

bool NpcSystem::is_valid_move( const sf::FloatRect &target_position )
{

  // arbitrary Cmp::NoPathFinding components
  auto nppathfinding_view = reg().view<Cmp::NpcNoPathFinding, Cmp::Position>();
  for ( auto [entt, nopath_cmp, pos_cmp] : nppathfinding_view.each() )
  {
    if ( pos_cmp.findIntersection( target_position ) )
    {
      SPDLOG_DEBUG( "NPC collided Cmp::NoPathFinding" );
      return false;
    }
  }

  return true;
}

void NpcSystem::check_once_collision()
{
  auto player_collision_view = reg().view<Cmp::PlayerCharacter>();
  auto npc_collision_view = reg().view<Cmp::NPC, Cmp::Position, Cmp::Direction>( entt::exclude<Cmp::NpcFriendly> );

  auto &player_dmg_cooldown = Sys::PersistSystem::get<Cmp::Persist::PcDamageDelay>( reg() );
  auto &player_pos = Utils::Player::get_position( reg() );
  auto &player_mort = Utils::Player::get_mortality( reg() );

  for ( auto [player_entity, player_cmp] : player_collision_view.each() )
  {
    if ( player_mort.state != Cmp::PlayerMortality::State::ALIVE ) return;
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp, npc_dir_cmp] : npc_collision_view.each() )
    {
      if ( not Utils::is_visible_in_view( RenderSystem::get_world_view(), npc_pos_cmp ) ) continue;
      if ( player_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < player_dmg_cooldown.get_value() ) continue;

      auto npc_collision_action = npc_cmp.actions.at( std::type_index( typeid( Cmp::CollisionAction ) ) );
      auto &[action, timer] = npc_collision_action;
      if ( action.interval() > 0.f ) continue;

      // relaxed bounds to allow player to sneak past during lerp transition
      auto npc_pos_cmp_bounds_current = Cmp::RectBounds::scaled( npc_pos_cmp.position, npc_pos_cmp.size, 0.1f );
      if ( not player_pos.findIntersection( npc_pos_cmp_bounds_current.getBounds() ) ) continue;

      Utils::Player::get_player_stats( reg() ).apply_modifiers( action );

      m_sound_bank.get_effect( "damage_player" ).play();

      if ( Utils::Player::get_player_stats( reg() ).health() <= 0 )
      {
        player_mort.state = Cmp::PlayerMortality::State::HAUNTED;
        get_systems_event_queue().enqueue(
            Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::HAUNTED, Utils::Player::get_position( reg() ) ) );
        return;
      }

      player_cmp.m_damage_cooldown_timer.restart();

      find_pushback_position( npc_dir_cmp );
    }
  }
}

void NpcSystem::check_timed_collision( sf::Time dt )
{
  auto npc_collision_view = reg().view<Cmp::NPC, Cmp::Position>( entt::exclude<Cmp::NpcFriendly> );

  auto &player_pos = Utils::Player::get_position( reg() );
  auto &player_mort = Utils::Player::get_mortality( reg() );

  if ( player_mort.state != Cmp::PlayerMortality::State::ALIVE ) return;
  for ( auto [npc_entity, npc_cmp, npc_pos_cmp] : npc_collision_view.each() )
  {
    if ( not Utils::is_visible_in_view( RenderSystem::get_world_view(), npc_pos_cmp ) ) continue;
    auto &npc_collision_action = npc_cmp.actions.at( std::type_index( typeid( Cmp::CollisionAction ) ) );

    auto &[npc_action, npc_action_timer] = npc_collision_action;
    if ( npc_action.interval() == 0.f ) continue;
    npc_action_timer += dt;
    if ( npc_action_timer.asSeconds() < npc_action.interval() ) continue;

    if ( not player_pos.findIntersection( npc_pos_cmp ) ) continue;

    Utils::Player::get_player_stats( reg() ).apply_modifiers( npc_action );
    if ( m_sound_bank.get_effect( "damage_player" ).getStatus() != sf::Sound::Status::Playing ) { m_sound_bank.get_effect( "damage_player" ).play(); }

    npc_action_timer = sf::Time::Zero;

    if ( Utils::Player::get_player_stats( reg() ).health() <= 0 )
    {
      player_mort.state = Cmp::PlayerMortality::State::HAUNTED;
      get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::HAUNTED, Utils::Player::get_position( reg() ) ) );
      return;
    }
  }
}

void NpcSystem::find_pushback_position( const Cmp::Direction &npc_direction )
{
  auto &player_pos = Utils::Player::get_position( reg() );

  auto new_position = Utils::snap_to_grid( player_pos.position + ( npc_direction.componentWiseMul( Constants::kGridSizePxF ) ) );
  SPDLOG_DEBUG( "Player position was {},{} - Knockback direction is {}, {} - New Position should be {},{}", player_pos.position.x,
                player_pos.position.y, npc_direction.x, npc_direction.y, new_position.x, new_position.y );

  // make sure player isnt knocked into an obstacle or multiblock
  auto new_pos_rect = Cmp::RectBounds::scaled( new_position, Constants::kGridSizePxF, 1.f );
  if ( Utils::Collision::check_cmp<Cmp::Obstacle>( reg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::AltarSegment>( reg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::GraveSegment>( reg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::CryptSegment>( reg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::RuinSegment>( reg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::CryptObjectiveSegment>( reg(), new_pos_rect ) ) return;

  player_pos.position = new_position;
}

void NpcSystem::update_shockwaves( sf::Time dt )
{
  // emit shockwaves from each NPC
  for ( auto [npc_entt, npc_cmp, anim_cmp, npc_pos_cmp, npc_uuid_cmp] : reg().view<Cmp::NPC, Cmp::AnimData, Cmp::Position, Cmp::UUID>().each() )
  {
    if ( anim_cmp.m_sprite_type == "sprite.priest" )
    {
      // cooldown is handled in Factory function via Cmp::NpcShockwaveTimer per NPC
      auto created_shockwave = Factory::create_shockwave( reg(), npc_entt );
      if ( created_shockwave )
      {
        Particle::Factory::add_shockwave( reg(), "particle.shockwave.priest", npc_uuid_cmp, npc_pos_cmp.getCenter(), 50000 );
      }

      // update the shockwave position so that it follows NPC
      Particle::Factory::update_position( reg(), npc_uuid_cmp, npc_pos_cmp.getCenter() );
    }
  }

  // Invert the interval - larger values = faster updates, smaller values = slower updates
  auto speed_value = Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveSpeed>( reg() ).get_value();
  auto max_radius = Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveMaxRadius>( reg() );

  for ( auto entt : reg().view<Cmp::NpcShockwave>() )
  {
    auto &sw_cmp = reg().get<Cmp::NpcShockwave>( entt );
    float current_radius = sw_cmp.sprite.get_radius();

    // Exponential scaling - shockwave accelerates as it grows
    // This creates a natural acceleration that maintains the visual impression of constant speed as the circumference grows.
    float normalized_radius = current_radius / max_radius.get_value();
    float speed_multiplier = 1.0f + ( normalized_radius * normalized_radius ); // Quadratic acceleration
    float new_radius = current_radius + ( speed_value * speed_multiplier * dt.asSeconds() );
    sw_cmp.sprite.set_radius( new_radius );

    check_shockwave_obstacle_collision( entt, sw_cmp );

    if ( new_radius > max_radius.get_value() ) { reg().destroy( entt ); }
  }
  shockwave_update_clock.restart();

  // Particle::Factory::delete_expired_particle_sprites( reg(), "particle.shockwave.priest" );
}

void NpcSystem::check_shockwave_obstacle_collision( [[maybe_unused]] entt::entity shockwave_entity, Cmp::NpcShockwave &shockwave )
{
  auto obstacle_view = reg().view<Cmp::Obstacle, Cmp::Position, Cmp::AnimData>();

  for ( auto [obstacle_entity, obstacle_cmp, obstacle_pos, sprite_anim] : obstacle_view.each() )
  {
    // Check if this is the specific obstacle type and index we care about
    if ( ( sprite_anim.m_sprite_type == "sprite.crypt.wall.int" && sprite_anim.getFrameIndexOffset() == 1 ) or
         ( sprite_anim.m_sprite_type == "sprite.crypt.wall.int" && sprite_anim.getFrameIndexOffset() == 0 ) )
    {
      sf::FloatRect obstacle_rect( obstacle_pos.position, obstacle_pos.size );

      SPDLOG_DEBUG( "Checking obstacle at ({}, {}) size ({}, {}) against shockwave at ({}, {}) radius {}", obstacle_rect.position.x,
                    obstacle_rect.position.y, obstacle_rect.size.x, obstacle_rect.size.y, shockwave.sprite.getPosition().x,
                    shockwave.sprite.getPosition().y, shockwave.sprite.getRadius() );

      // Calculate distance from circle center to rectangle
      sf::Vector2f circle_center = shockwave.sprite.get_position();
      float circle_radius = shockwave.sprite.get_radius();

      // Find closest point on rectangle to circle center
      sf::Vector2f closest_point;
      closest_point.x = std::max( obstacle_rect.position.x, std::min( circle_center.x, obstacle_rect.position.x + obstacle_rect.size.x ) );
      closest_point.y = std::max( obstacle_rect.position.y, std::min( circle_center.y, obstacle_rect.position.y + obstacle_rect.size.y ) );

      // Calculate distance from circle center to closest point
      sf::Vector2f diff = circle_center - closest_point;
      float distance = std::sqrt( ( diff.x * diff.x ) + ( diff.y * diff.y ) );

      // Check if circle intersects with rectangle (accounting for outline thickness)
      float effective_radius = circle_radius + ( shockwave.sprite.get_outline_thickness() / 2.0f );

      if ( distance <= effective_radius )
      {
        SPDLOG_DEBUG( "Shockwave INTERSECTS with obstacle (distance: {}, effective_radius: {})", distance, effective_radius );
        Sys::ShockwaveSystem::remove_intersecting_segments( obstacle_rect, shockwave );
      }
      else { SPDLOG_DEBUG( "Shockwave does NOT intersect with obstacle (distance: {}, effective_radius: {})", distance, effective_radius ); }
    }
  }
}

} // namespace Game::Sys