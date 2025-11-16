#include <Components/GraveSegment.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ShrineSegment.hpp>
#include <Components/WormholeJump.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Door.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/WormholeSeed.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Components/Wormhole.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/WormholeSystem.hpp>

namespace ProceduralMaze::Sys
{

WormholeSystem::WormholeSystem( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( window, sprite_factory, sound_bank )
{

  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
  getEventDispatcher().sink<Events::PauseClocksEvent>().connect<&Sys::WormholeSystem::onPause>( this );
  getEventDispatcher().sink<Events::ResumeClocksEvent>().connect<&Sys::WormholeSystem::onResume>( this );

  SPDLOG_DEBUG( "WormholeSystem initialized" );
}

void WormholeSystem::onPause()
{
  if ( m_sound_bank.get_effect( "wormhole_jump" ).getStatus() == sf::Sound::Status::Playing )
    m_sound_bank.get_effect( "wormhole_jump" ).pause();

  auto jump_view = m_reg->view<Cmp::WormholeJump>();
  for ( auto [entity, jump_cmp] : jump_view.each() )
  {
    jump_cmp.jump_clock.stop();
  }
}

void WormholeSystem::onResume()
{
  if ( m_sound_bank.get_effect( "wormhole_jump" ).getStatus() == sf::Sound::Status::Paused )
    m_sound_bank.get_effect( "wormhole_jump" ).play();

  auto jump_view = m_reg->view<Cmp::WormholeJump>();
  for ( auto [entity, jump_cmp] : jump_view.each() )
  {
    jump_cmp.jump_clock.start();
  }
}

std::pair<entt::entity, Cmp::Position> WormholeSystem::find_spawn_location( unsigned long seed )
{
  constexpr int kMaxAttempts = 1000;
  int attempts = 0;
  unsigned long current_seed = seed;

  while ( attempts < kMaxAttempts )
  {
    auto [random_entity, random_pos] = get_random_position(
        IncludePack<Cmp::Obstacle>{},
        ExcludePack<Cmp::Wall, Cmp::Door, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC, Cmp::ReservedPosition>{}, current_seed );

    Cmp::RectBounds wormhole_hitbox( random_pos.position, random_pos.size, 2.f );

    // Check collisions with walls, graves, shrines
    auto is_valid = [&]() -> bool
    {
      // return false for wall collisions
      for ( auto [entity, wall_cmp, wall_pos_cmp] : m_reg->view<Cmp::Wall, Cmp::Position>().each() )
      {
        if ( wall_pos_cmp.findIntersection( wormhole_hitbox.getBounds() ) ) return false;
      }

      // Return false for grave collisions
      for ( auto [entity, grave_cmp, grave_pos_cmp] : m_reg->view<Cmp::GraveSegment, Cmp::Position>().each() )
      {
        if ( grave_pos_cmp.findIntersection( wormhole_hitbox.getBounds() ) ) return false;
      }

      // Return false for shrine collisions
      for ( auto [entity, shrine_cmp, shrine_pos_cmp] : m_reg->view<Cmp::ShrineSegment, Cmp::Position>().each() )
      {
        if ( shrine_pos_cmp.findIntersection( wormhole_hitbox.getBounds() ) ) return false;
      }

      return true;
    };

    if ( is_valid() )
    {
      if ( current_seed != seed && seed > 0 )
      {
        SPDLOG_WARN( "Wormhole spawn: original seed {} was invalid, used seed {} instead (attempt {})", seed, current_seed,
                     attempts + 1 );
      }
      return { random_entity, random_pos };
    }

    attempts++;
    // Increment seed for next attempt (works for both seeded and non-seeded cases)
    if ( seed > 0 ) { current_seed++; }
  }

  SPDLOG_ERROR( "Failed to find valid wormhole spawn location after {} attempts (original seed: {})", kMaxAttempts, seed );
  return { entt::null, Cmp::Position{ { 0.f, 0.f }, { 0.f, 0.f } } };
}

void WormholeSystem::spawn_wormhole( SpawnPhase phase )
{
  // 1. pick a random position component in the maze, exclude walls, doors, exits, and playable characters
  // 2. get the entity at that position
  unsigned long seed = 0;
  if ( phase == SpawnPhase::InitialSpawn ) seed = get_persistent_component<Cmp::Persistent::WormholeSeed>().get_value();

  auto [random_entity, random_pos] = find_spawn_location( seed );
  if ( random_entity == entt::null )
  {
    SPDLOG_ERROR( "Failed to find valid wormhole spawn position." );
    return;
  }

  // 3. set the entities obstacle component to "broken" so we have something for the shader effect to mangle
  auto random_pos_3x3_hitbox = Cmp::RectBounds( random_pos.position, random_pos.size, 3.f );
  auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>();
  for ( auto [entity, obstacle_cmp, obstacle_pos_cmp] : obstacle_view.each() )
  {
    if ( obstacle_pos_cmp.findIntersection( random_pos_3x3_hitbox.getBounds() ) )
    {
      obstacle_cmp.m_enabled = false;
      SPDLOG_INFO( "Wormhole spawn: Destroying obstacle at ({}, {})", obstacle_pos_cmp.position.x, obstacle_pos_cmp.position.y );
    }
  }

  // 4. add the wormhole component to the entity
  m_reg->emplace_or_replace<Cmp::Wormhole>( random_entity );
  m_reg->emplace_or_replace<Cmp::SpriteAnimation>( random_entity, 0, 0, true, "WORMHOLE" );

  SPDLOG_INFO( "Wormhole spawned at position ({}, {})", random_pos.position.x, random_pos.position.y );
}

void WormholeSystem::check_player_wormhole_collision()
{
  auto wormhole_view = m_reg->view<Cmp::Wormhole, Cmp::Position>();
  auto all_actors_view = m_reg->view<Cmp::Direction, Cmp::Position>();

  // First, check for any entities with WormholeJump that are NOT colliding
  auto jump_view = m_reg->view<Cmp::WormholeJump>();
  for ( auto [entity, jump_cmp] : jump_view.each() )
  {
    bool still_colliding = false;

    auto *jump_pos_cmp = m_reg->try_get<Cmp::Position>( entity );
    // TODO: pointless check? Never happens (according to log)
    if ( !jump_pos_cmp )
    {
      SPDLOG_INFO( "Entity {} has WormholeJump but NO Position component - removing jump", static_cast<uint32_t>( entity ) );
      m_reg->remove<Cmp::WormholeJump>( entity );
      continue;
    }

    for ( auto [wormhole_entity, wormhole_cmp, wh_pos_cmp] : wormhole_view.each() )
    {
      auto wh_hitbox_redux = Cmp::RectBounds( wh_pos_cmp.position, wh_pos_cmp.size, 0.5f );
      if ( jump_pos_cmp && jump_pos_cmp->findIntersection( wh_hitbox_redux.getBounds() ) )
      {
        still_colliding = true;
        break;
      }
    }

    if ( !still_colliding )
    {
      SPDLOG_WARN( "Entity {} has WormholeJump but is NO LONGER colliding - removing jump component",
                   static_cast<uint32_t>( entity ) );
      m_reg->remove<Cmp::WormholeJump>( entity );
      m_sound_bank.get_effect( "wormhole_jump" ).stop();
    }
  }

  // Now check for new/ongoing collisions and collect entities ready to teleport
  for ( auto [actor_entity, actor_dir_cmp, actor_pos_cmp] : all_actors_view.each() )
  {
    for ( auto [wormhole_entity, wormhole_cmp, wh_pos_cmp] : wormhole_view.each() )
    {

      auto wh_hitbox_redux = Cmp::RectBounds( wh_pos_cmp.position, wh_pos_cmp.size, 0.5f );
      if ( !actor_pos_cmp.findIntersection( wh_hitbox_redux.getBounds() ) ) continue;

      // Check if jump component already exists
      auto *wh_jump_cmp = m_reg->try_get<Cmp::WormholeJump>( actor_entity );
      if ( !wh_jump_cmp )
      {
        // First collision - create component
        m_reg->emplace<Cmp::WormholeJump>( actor_entity );
        SPDLOG_INFO( "Entity {} is jump candidate.", static_cast<uint32_t>( actor_entity ) );
        // restart the jump sfx for each actor processed so that it is heard by the last actor.
        // There is adequate lead time on the sfx (~2secs) to prevent restart stuttering.
        m_sound_bank.get_effect( "wormhole_jump" ).play();
      }
    }
  }

  // Count how many entitiesare ready to teleport
  std::size_t teleport_ready_count = 0;
  for ( auto [entity, jump_cmp] : jump_view.each() )
  {
    // Check if cooldown complete
    float elapsed = jump_cmp.jump_clock.getElapsedTime().asSeconds();
    float cooldown = jump_cmp.jump_cooldown.asSeconds();

    if ( elapsed >= cooldown ) { teleport_ready_count++; }
  }

  // Commence teleportation if all entities are ready to jump
  if ( teleport_ready_count == jump_view.size() && teleport_ready_count > 0 )
  {
    SPDLOG_INFO( "Teleportation commencing. Jump candidates: {}", jump_view.size() );
    for ( auto [entity, jump_cmp] : jump_view.each() )
    {

      // Get unique random position for this actor entity
      auto [new_spawn_entity, new_spawn_pos_cmp] = get_random_position(
          IncludePack<Cmp::Obstacle>{}, ExcludePack<Cmp::Wall, Cmp::Door, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC>{}, 0 );

      // destroy any obstacles at the new actor spawn position
      auto existing_obstacle_cmp = m_reg->try_get<Cmp::Obstacle>( new_spawn_entity );
      if ( existing_obstacle_cmp )
      {
        SPDLOG_INFO( "Entity {} - Removing obstacle at new spawn ({}, {})", static_cast<uint32_t>( entity ),
                     new_spawn_pos_cmp.position.x, new_spawn_pos_cmp.position.y );
        existing_obstacle_cmp->m_integrity = 0.0f;
      }

      // update the teleported entity's components
      SPDLOG_INFO( "Entity {} - TELEPORTING NOW!", static_cast<uint32_t>( entity ) );
      m_reg->remove<Cmp::LerpPosition>( entity );
      m_reg->emplace_or_replace<Cmp::Position>( entity, new_spawn_pos_cmp.position, new_spawn_pos_cmp.size );
      m_reg->remove<Cmp::WormholeJump>( entity );
      auto *npc_scan_bounds = m_reg->try_get<Cmp::NPCScanBounds>( entity );
      if ( npc_scan_bounds ) { npc_scan_bounds->position( new_spawn_pos_cmp.position ); }

      SPDLOG_INFO( "Entity {} - TELEPORT to ({}, {}) COMPLETE", static_cast<uint32_t>( entity ), new_spawn_pos_cmp.position.x,
                   new_spawn_pos_cmp.position.y );
    }

    // respawn the wormhole now all entities have teleported
    SPDLOG_INFO( "Teleportation complete. Jump candidates: {}", jump_view.size() );
    despawn_wormhole();
    spawn_wormhole( WormholeSystem::SpawnPhase::Respawn );
  }
}

void WormholeSystem::despawn_wormhole()
{
  // remove the wormhole entity
  auto wormhole_view = m_reg->view<Cmp::Wormhole>();
  for ( auto [entity, _] : wormhole_view.each() )
  {
    m_reg->remove<Cmp::Wormhole>( entity );
    SPDLOG_INFO( "Wormhole despawned (entity {})", static_cast<uint32_t>( entity ) );
  }
}

} // namespace ProceduralMaze::Sys