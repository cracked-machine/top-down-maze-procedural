#include <Audio/SoundBank.hpp>
#include <Collision.hpp>
#include <Components/Altar/AltarSegment.hpp>
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
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcShockwaveMaxRadius.hpp>
#include <Components/Persistent/NpcShockwaveSpeed.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerHealth.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/RectBounds.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Components/Wormhole/WormholeJump.hpp>
#include <Components/ZOrderValue.hpp>
#include <Crypt/CryptObjectiveSegment.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Factory/NpcFactory.hpp>
#include <Grave/GraveSegment.hpp>
#include <Npc/NpcFriendly.hpp>
#include <Npc/NpcLerpSpeed.hpp>
#include <PathFinding/AStar.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Ruin/RuinSegment.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>
#include <Systems/Threats/ShockwaveSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Npc.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Time.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys
{

NpcSystem::NpcSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "NpcSystem initialized" );
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

  // run pathfinding at 10Hz
  static constexpr float kScanInterval = 0.10f;
  m_scan_accumulator += dt;
  if ( m_scan_accumulator.asSeconds() >= kScanInterval )
  {
    update_pathfinding( Utils::Player::get_entity( getReg() ) );
    m_scan_accumulator = sf::Time::Zero;
  }

  // run animation checks at 20Hz
  static constexpr float kAnimationInterval = 0.05f;
  m_animation_accumulator += dt;
  if ( m_animation_accumulator.asSeconds() >= kAnimationInterval )
  {
    update_animation();
    m_animation_accumulator = sf::Time::Zero;
  }

  update_movement( dt );

  if ( not Utils::getSystemCmp( getReg() ).collisions_disabled ) { check_player_to_npc_collision(); }

  update_shockwaves();
}

void NpcSystem::check_bones_reanimation()
{

  auto player_pos = Utils::Player::get_position( getReg() );
  auto npccontainer_collision_view = getReg().view<Cmp::NpcContainer, Cmp::Position>();

  for ( auto [npccontainer_entt, npccontainer_cmp, npccontainer_pos_cmp] : npccontainer_collision_view.each() )
  {
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), npccontainer_pos_cmp ) ) continue;

    auto &npc_activate_scale = Sys::PersistSystem::get<Cmp::Persist::NpcActivateScale>( getReg() );
    // we just create a temporary RectBounds here instead of a component because we only need it
    // for this one comparison and it already contains the needed scaling logic
    auto npc_activate_bounds = Cmp::RectBounds::scaled( npccontainer_pos_cmp.position, Constants::kGridSizePxF, npc_activate_scale.get_value() );

    if ( player_pos.findIntersection( npc_activate_bounds.getBounds() ) )
    {
      Factory::create_npc( getReg(), npccontainer_entt, "NPCSKELE" );
      m_sound_bank.get_effect( "spawn_skeleton" ).play();
    }
  }
}

void NpcSystem::update_animation()
{
  for ( auto [npc_entt, npc_cmp, npc_dir_cmp, anim_cmp] : getReg().view<Cmp::NPC, Cmp::Direction, Cmp::SpriteAnimation>().each() )
  {

    if ( npc_dir_cmp == sf::Vector2f( 0.0f, 0.0f ) )
    {
      anim_cmp.m_animation_active = false;
      continue;
    }
    else
    {
      anim_cmp.m_animation_active = true;
      if ( anim_cmp.m_sprite_type.contains( "NPCSKELE" ) )
      {

        if ( npc_dir_cmp.x > 0 ) { anim_cmp.m_sprite_type = "NPCSKELE.walk.east"; }
        else if ( npc_dir_cmp.x < 0 ) { anim_cmp.m_sprite_type = "NPCSKELE.walk.west"; }
        else if ( npc_dir_cmp.y < 0 ) { anim_cmp.m_sprite_type = "NPCSKELE.walk.north"; }
        else if ( npc_dir_cmp.y > 0 ) { anim_cmp.m_sprite_type = "NPCSKELE.walk.south"; }
      }
      else if ( anim_cmp.m_sprite_type.contains( "NPCGHOST" ) )
      {
        // Ghost NPCs face cardinal directions only
        if ( npc_dir_cmp.x > 0 ) { anim_cmp.m_sprite_type = "NPCGHOST.walk.east"; }
        else if ( npc_dir_cmp.x < 0 ) { anim_cmp.m_sprite_type = "NPCGHOST.walk.west"; }
        else if ( npc_dir_cmp.y < 0 ) { anim_cmp.m_sprite_type = "NPCGHOST.walk.north"; }
        else if ( npc_dir_cmp.y > 0 ) { anim_cmp.m_sprite_type = "NPCGHOST.walk.south"; }
      }
    }
  }
}

void NpcSystem::update_pathfinding( [[maybe_unused]] entt::entity player_entity )
{

  PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock();
  if ( not pathfinding_navmesh ) return;

  auto player_pos_cmp = Utils::Player::get_position( getReg() );
  auto player_in_spawn = Utils::Player::is_in_spawn( getReg(), player_pos_cmp );

  auto npc_view = getReg().view<Cmp::NPC, Cmp::Position, Cmp::SpriteAnimation, Cmp::NpcLerpSpeed>( entt::exclude<Cmp::NpcFriendly> );
  for ( auto [npc_entity, npc_cmp, npc_pos_cmp, anim_cmp, lerp_speed_cmp] : npc_view.each() )
  {

    if ( not Utils::is_visible_in_view( RenderSystem::getGameView(), npc_pos_cmp ) ) continue;

    // don't intterupt NPC mid-lerp or it causes indecisive pathfinding
    auto *npc_lerp_pos_cmp = getReg().try_get<Cmp::LerpPosition>( npc_entity );
    if ( npc_lerp_pos_cmp && npc_lerp_pos_cmp->m_lerp_factor < 1.0f ) continue;

    // allow ghosts to sneak through gaps
    auto query_compass = PathFinding::QueryCompass::CARDINAL;
    if ( anim_cmp.m_sprite_type.contains( "NPCGHOST" ) ) query_compass = PathFinding::QueryCompass::BOTH;

    // now get latest path vector for NPC -> player
    std::vector<PathFinding::PathNode> path = PathFinding::astar( getReg(), *pathfinding_navmesh, npc_pos_cmp, player_pos_cmp, query_compass );

    // dont let NPC follow player into spawn but keep pathfinding active up to penultimate path node
    // if ( player_in_spawn and not path.empty() ) path.pop_back();
    if ( path.size() > 1 )
    {
      // path[0] is the NPC current position, so go one forward
      auto new_position_cmp = path[1].pos;

      // If the player is in spawn, pathfind to them but not to final position
      if ( player_in_spawn and player_pos_cmp == new_position_cmp ) continue;

      // calculate the direction and update the NPC lerp
      auto candidate_lerp_pos = Cmp::LerpPosition( new_position_cmp.position, lerp_speed_cmp.speed );
      auto distance_to_target = new_position_cmp.position - npc_pos_cmp.position;
      if ( distance_to_target == sf::Vector2f( 0.0f, 0.0f ) ) continue;

      // prevent NPC warping via another NPCs pathfinding
      const bool too_far = std::abs( distance_to_target.x ) >= Constants::kGridSizePxF.x * 1.5f ||
                           std::abs( distance_to_target.y ) >= Constants::kGridSizePxF.y * 1.5f;
      if ( too_far ) continue;

      auto norm_direction = Cmp::Direction( distance_to_target.normalized() );

      getReg().emplace_or_replace<Cmp::Direction>( npc_entity, std::move( norm_direction ) );
      getReg().emplace_or_replace<Cmp::LerpPosition>( npc_entity, std::move( candidate_lerp_pos ) );
    }
  }
}

void NpcSystem::update_movement( sf::Time dt )
{
  auto exclusions = entt::exclude<Cmp::AltarSegment, Cmp::CryptSegment, Cmp::SpawnArea, Cmp::PlayerCharacter>;
  auto view = getReg().view<Cmp::Position, Cmp::LerpPosition, Cmp::Direction>( exclusions );

  for ( auto [npc_entt, pos_cmp, lerp_pos_cmp, dir_cmp] : view.each() )
  {

    // if there is an obstacle at this entity move onto the next entity
    auto obst_cmp = getReg().try_get<Cmp::Obstacle>( npc_entt );
    if ( obst_cmp ) continue;

    // If this is the first update, store the start position
    if ( lerp_pos_cmp.m_lerp_factor == 0.0f )
    {
      // Allow NPCs to escape wormholes if they're mid-lerp.
      if ( getReg().try_get<Cmp::WormholeJump>( npc_entt ) ) continue;

      lerp_pos_cmp.m_start = pos_cmp.position;
    }

    lerp_pos_cmp.m_lerp_factor += lerp_pos_cmp.m_lerp_speed * dt.asSeconds();

    // lerp has completed
    if ( lerp_pos_cmp.m_lerp_factor >= 1.0f )
    {
      auto old_position = pos_cmp;
      pos_cmp.position = lerp_pos_cmp.m_target;
      getReg().remove<Cmp::LerpPosition>( npc_entt );
      if ( PathFinding::SpatialHashGridSharedPtr pathfinding_navmesh = m_pathfinding_navmesh.lock() )
        pathfinding_navmesh->update( npc_entt, old_position, pos_cmp );
    }
    else
    {
      // Lerp from start to target directly
      // Simple manual lerp - 33 lines of assembly vs 134 for std::lerp vs 54 for std::fma - https://godbolt.org/z/YdeKco5d6
      const float t = lerp_pos_cmp.m_lerp_factor;
      const float one_minus_t = 1.0f - t;
      pos_cmp.position.x = one_minus_t * lerp_pos_cmp.m_start.x + t * lerp_pos_cmp.m_target.x;
      pos_cmp.position.y = one_minus_t * lerp_pos_cmp.m_start.y + t * lerp_pos_cmp.m_target.y;
    }

    getReg().patch<Cmp::ZOrderValue>( npc_entt, [&]( auto &zorder_cmp ) { zorder_cmp.setZOrder( pos_cmp.position.y ); } );
  }
}

bool NpcSystem::is_valid_move( const sf::FloatRect &target_position )
{

  // arbitrary Cmp::NoPathFinding components
  auto nppathfinding_view = getReg().view<Cmp::NpcNoPathFinding, Cmp::Position>();
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

void NpcSystem::check_player_to_npc_collision()
{
  auto player_collision_view = getReg().view<Cmp::PlayerCharacter>();
  auto npc_collision_view = getReg().view<Cmp::NPC, Cmp::Position, Cmp::Direction>( entt::exclude<Cmp::NpcFriendly> );

  auto &player_dmg_cooldown = Sys::PersistSystem::get<Cmp::Persist::PcDamageDelay>( getReg() );
  auto &player_pos = Utils::Player::get_position( getReg() );
  auto &player_mort = Utils::Player::get_mortality( getReg() );
  auto &player_health = Utils::Player::get_health( getReg() );

  for ( auto [pc_entity, pc_cmp] : player_collision_view.each() )
  {
    if ( player_mort.state != Cmp::PlayerMortality::State::ALIVE ) return;
    for ( auto [npc_entity, npc_cmp, npc_pos_cmp, npc_dir_cmp] : npc_collision_view.each() )
    {
      if ( not Utils::is_visible_in_view( RenderSystem::getGameView(), npc_pos_cmp ) ) continue;

      // relaxed bounds to allow player to sneak past during lerp transition
      auto npc_pos_cmp_bounds_current = Cmp::RectBounds::scaled( npc_pos_cmp.position, npc_pos_cmp.size, 0.1f );
      if ( not player_pos.findIntersection( npc_pos_cmp_bounds_current.getBounds() ) ) continue;

      if ( pc_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < player_dmg_cooldown.get_value() ) continue;

      auto &npc_damage = Sys::PersistSystem::get<Cmp::Persist::NpcDamage>( getReg() );
      player_health.health -= npc_damage.get_value();

      m_sound_bank.get_effect( "damage_player" ).play();

      if ( player_health.health <= 0 )
      {
        player_mort.state = Cmp::PlayerMortality::State::HAUNTED;
        get_systems_event_queue().enqueue(
            Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::HAUNTED, Utils::Player::get_position( getReg() ) ) );
        return;
      }

      pc_cmp.m_damage_cooldown_timer.restart();

      find_pushback_position( npc_dir_cmp );
    }
  }
}

void NpcSystem::find_pushback_position( const Cmp::Direction &npc_direction )
{
  auto &player_pos = Utils::Player::get_position( getReg() );

  auto new_position = Utils::snap_to_grid( player_pos.position + ( npc_direction.componentWiseMul( Constants::kGridSizePxF ) ) );
  SPDLOG_DEBUG( "Player position was {},{} - Knockback direction is {}, {} - New Position should be {},{}", player_pos.position.x,
                player_pos.position.y, npc_direction.x, npc_direction.y, new_position.x, new_position.y );

  // make sure player isnt knocked into an obstacle or multiblock
  auto new_pos_rect = Cmp::RectBounds::scaled( new_position, Constants::kGridSizePxF, 1.f );
  if ( Utils::Collision::check_cmp<Cmp::Obstacle>( getReg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::AltarSegment>( getReg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::GraveSegment>( getReg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::CryptSegment>( getReg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::RuinSegment>( getReg(), new_pos_rect ) ) return;
  if ( Utils::Collision::check_cmp<Cmp::CryptObjectiveSegment>( getReg(), new_pos_rect ) ) return;

  player_pos.position = new_position;
}

void NpcSystem::update_shockwaves()
{
  // emit shockwaves from each NPC
  for ( auto npc_entt : getReg().view<Cmp::NPC>() )
  {
    auto npc_sprite_anim = getReg().try_get<Cmp::SpriteAnimation>( npc_entt );
    if ( npc_sprite_anim && npc_sprite_anim->m_sprite_type == "NPCPRIEST" )
    {
      // cooldown is handled in Factory function via Cmp::NpcShockwaveTimer per NPC
      Factory::create_shockwave( getReg(), npc_entt );
    }
  }

  auto shockwave_increments = 1;

  // Invert the interval - larger values = faster updates, smaller values = slower updates
  auto speed_value = Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveSpeed>( getReg() ).get_value();
  sf::Time shockwave_update_interval{ sf::milliseconds( static_cast<int>( 1000.0f / speed_value ) ) };

  auto max_radius = Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveMaxRadius>( getReg() );

  if ( shockwave_update_clock.getElapsedTime() > shockwave_update_interval )
  {
    for ( auto entt : getReg().view<Cmp::NpcShockwave>() )
    {
      auto &sw_cmp = getReg().get<Cmp::NpcShockwave>( entt );
      float current_radius = sw_cmp.sprite.getRadius();

      // Exponential scaling - shockwave accelerates as it grows
      // This creates a natural acceleration that maintains the visual impression of constant speed as the circumference grows.
      float normalized_radius = current_radius / max_radius.get_value();
      float speed_multiplier = 1.0f + normalized_radius * normalized_radius; // Quadratic acceleration

      float new_radius = current_radius + ( shockwave_increments * speed_multiplier );
      sw_cmp.sprite.setRadius( new_radius );

      checkShockwaveObstacleCollision( entt, sw_cmp );

      if ( new_radius > max_radius.get_value() ) { getReg().destroy( entt ); }
    }
    shockwave_update_clock.restart();
  }
}

void NpcSystem::checkShockwaveObstacleCollision( [[maybe_unused]] entt::entity shockwave_entity, Cmp::NpcShockwave &shockwave )
{
  auto obstacle_view = getReg().view<Cmp::Obstacle, Cmp::Position, Cmp::SpriteAnimation>();

  for ( auto [obstacle_entity, obstacle_cmp, obstacle_pos, sprite_anim] : obstacle_view.each() )
  {
    // Check if this is the specific obstacle type and index we care about
    if ( ( sprite_anim.m_sprite_type == "CRYPT.interior_sb" && sprite_anim.getFrameIndexOffset() == 3 ) or
         ( sprite_anim.m_sprite_type == "CRYPT.interior_sb" && sprite_anim.getFrameIndexOffset() == 2 ) or
         ( sprite_anim.m_sprite_type == "CRYPT.interior_sb" && sprite_anim.getFrameIndexOffset() == 0 ) )
    {
      sf::FloatRect obstacle_rect( obstacle_pos.position, obstacle_pos.size );

      SPDLOG_DEBUG( "Checking obstacle at ({}, {}) size ({}, {}) against shockwave at ({}, {}) radius {}", obstacle_rect.position.x,
                    obstacle_rect.position.y, obstacle_rect.size.x, obstacle_rect.size.y, shockwave.sprite.getPosition().x,
                    shockwave.sprite.getPosition().y, shockwave.sprite.getRadius() );

      // Calculate distance from circle center to rectangle
      sf::Vector2f circle_center = shockwave.sprite.getPosition();
      float circle_radius = shockwave.sprite.getRadius();

      // Find closest point on rectangle to circle center
      sf::Vector2f closest_point;
      closest_point.x = std::max( obstacle_rect.position.x, std::min( circle_center.x, obstacle_rect.position.x + obstacle_rect.size.x ) );
      closest_point.y = std::max( obstacle_rect.position.y, std::min( circle_center.y, obstacle_rect.position.y + obstacle_rect.size.y ) );

      // Calculate distance from circle center to closest point
      sf::Vector2f diff = circle_center - closest_point;
      float distance = std::sqrt( diff.x * diff.x + diff.y * diff.y );

      // Check if circle intersects with rectangle (accounting for outline thickness)
      float effective_radius = circle_radius + shockwave.sprite.getOutlineThickness() / 2.0f;

      if ( distance <= effective_radius )
      {
        SPDLOG_DEBUG( "Shockwave INTERSECTS with obstacle (distance: {}, effective_radius: {})", distance, effective_radius );
        Sys::ShockwaveSystem::removeIntersectingSegments( obstacle_rect, shockwave );
      }
      else { SPDLOG_DEBUG( "Shockwave does NOT intersect with obstacle (distance: {}, effective_radius: {})", distance, effective_radius ); }
    }
  }
}

} // namespace ProceduralMaze::Sys