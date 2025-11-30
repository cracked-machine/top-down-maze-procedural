#include <Components/AltarSegment.hpp>
#include <Components/Exit.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/RectBounds.hpp>
#include <Components/WormholeJump.hpp>
#include <Components/WormholeMultiBlock.hpp>
#include <Components/WormholeSingularity.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/WormholeSeed.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/WormholeSystem.hpp>

namespace ProceduralMaze::Sys
{

WormholeSystem::WormholeSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{

  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
  get_systems_event_queue().sink<Events::PauseClocksEvent>().connect<&Sys::WormholeSystem::onPause>( this );
  get_systems_event_queue().sink<Events::ResumeClocksEvent>().connect<&Sys::WormholeSystem::onResume>( this );

  SPDLOG_DEBUG( "WormholeSystem initialized" );
}

void WormholeSystem::onPause()
{
  if ( m_sound_bank.get_effect( "wormhole_jump" ).getStatus() == sf::Sound::Status::Playing ) m_sound_bank.get_effect( "wormhole_jump" ).pause();

  auto jump_view = getReg().view<Cmp::WormholeJump>();
  for ( auto [entity, jump_cmp] : jump_view.each() )
  {
    jump_cmp.jump_clock.stop();
  }
}

void WormholeSystem::onResume()
{
  if ( m_sound_bank.get_effect( "wormhole_jump" ).getStatus() == sf::Sound::Status::Paused ) m_sound_bank.get_effect( "wormhole_jump" ).play();

  auto jump_view = getReg().view<Cmp::WormholeJump>();
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
        IncludePack<Cmp::Obstacle>{}, ExcludePack<Cmp::Wall, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC, Cmp::ReservedPosition>{}, current_seed );

    auto &wormhole_ms = m_sprite_factory.get_multisprite_by_type( "WORMHOLE" );
    Cmp::WormholeMultiBlock wormhole_block( random_pos.position, wormhole_ms.get_grid_size().componentWiseMul( BaseSystem::kGridSquareSizePixels ) );

    // Cmp::RectBounds wormhole_hitbox( random_pos.position, random_pos.size, 2.f );

    // Check collisions with walls, graves, shrines
    auto is_valid = [&]() -> bool
    {
      // return false for wall collisions
      for ( auto [entity, wall_cmp, wall_pos_cmp] : getReg().view<Cmp::Wall, Cmp::Position>().each() )
      {
        if ( wall_pos_cmp.findIntersection( wormhole_block ) ) return false;
      }

      // Return false for grave collisions
      for ( auto [entity, grave_cmp, grave_pos_cmp] : getReg().view<Cmp::GraveSegment, Cmp::Position>().each() )
      {
        if ( grave_pos_cmp.findIntersection( wormhole_block ) ) return false;
      }

      // Return false for shrine collisions
      for ( auto [entity, shrine_cmp, shrine_pos_cmp] : getReg().view<Cmp::AltarSegment, Cmp::Position>().each() )
      {
        if ( shrine_pos_cmp.findIntersection( wormhole_block ) ) return false;
      }

      return true;
    };

    if ( is_valid() )
    {
      if ( current_seed != seed && seed > 0 )
      {
        SPDLOG_WARN( "Wormhole spawn: original seed {} was invalid, used seed {} instead (attempt {})", seed, current_seed, attempts + 1 );
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
  auto &wormhole_ms = m_sprite_factory.get_multisprite_by_type( "WORMHOLE" );
  Cmp::WormholeMultiBlock wormhole_block( random_pos.position, wormhole_ms.get_grid_size().componentWiseMul( BaseSystem::kGridSquareSizePixels ) );

  auto obstacle_view = getReg().view<Cmp::Position>();
  for ( auto [entity, obstacle_pos] : obstacle_view.each() )
  {
    if ( obstacle_pos.findIntersection( wormhole_block ) )
    {
      if ( getReg().any_of<Cmp::ZOrderValue>( entity ) ) getReg().remove<Cmp::ZOrderValue>( entity );
      if ( getReg().any_of<Cmp::Obstacle>( entity ) )
      {
        auto &obstacle_cmp = getReg().get<Cmp::Obstacle>( entity );
        obstacle_cmp.m_enabled = false;
      }

      SPDLOG_INFO( "Wormhole spawn: Destroying obstacle at ({}, {})", obstacle_pos.position.x, obstacle_pos.position.y );
    }
  }

  // 4. add the wormhole component to the entity
  // get the erntity that owns the center grid position of the 3x3 area
  sf::Vector2f center_pos = random_pos.position + BaseSystem::kGridSquareSizePixelsF;
  auto center_entity = getReg().create();
  getReg().emplace<Cmp::Position>( center_entity, center_pos, BaseSystem::kGridSquareSizePixelsF );
  getReg().emplace<Cmp::WormholeSingularity>( center_entity );

  // getReg().emplace_or_replace<Cmp::WormholeSingularity>( random_entity );
  getReg().emplace_or_replace<Cmp::WormholeMultiBlock>( random_entity, random_pos.position,
                                                        wormhole_ms.get_grid_size().componentWiseMul( BaseSystem::kGridSquareSizePixels ) );
  getReg().emplace_or_replace<Cmp::SpriteAnimation>( random_entity, 0, 0, true, "WORMHOLE" );
  getReg().emplace_or_replace<Cmp::ZOrderValue>( random_entity, random_pos.position.y - random_pos.size.y );

  SPDLOG_INFO( "Wormhole spawned at position ({}, {})", random_pos.position.x, random_pos.position.y );
}

void WormholeSystem::check_player_wormhole_collision()
{
  auto wormhole_view = getReg().view<Cmp::WormholeSingularity, Cmp::Position>();
  auto all_actors_view = getReg().view<Cmp::Direction, Cmp::Position>();

  // First, check for any entities with WormholeJump that are NOT colliding
  auto jump_view = getReg().view<Cmp::WormholeJump>();
  for ( auto [entity, jump_cmp] : jump_view.each() )
  {
    bool still_colliding = false;

    auto *jump_pos_cmp = getReg().try_get<Cmp::Position>( entity );
    // TODO: pointless check? Never happens (according to log)
    if ( !jump_pos_cmp )
    {
      SPDLOG_INFO( "Entity {} has WormholeJump but NO Position component - removing jump", static_cast<uint32_t>( entity ) );
      getReg().remove<Cmp::WormholeJump>( entity );
      continue;
    }

    for ( auto [wormhole_entity, wormhole_cmp, wh_pos_cmp] : wormhole_view.each() )
    {
      auto wh_hitbox_redux = Cmp::RectBounds( wh_pos_cmp.position, wh_pos_cmp.size, 1.f );
      if ( jump_pos_cmp && jump_pos_cmp->findIntersection( wh_hitbox_redux.getBounds() ) )
      {
        still_colliding = true;
        break;
      }
    }

    if ( !still_colliding )
    {
      SPDLOG_WARN( "Entity {} has WormholeJump but is NO LONGER colliding - removing jump component", static_cast<uint32_t>( entity ) );
      getReg().remove<Cmp::WormholeJump>( entity );
      m_sound_bank.get_effect( "wormhole_jump" ).stop();
    }
  }

  // Now check for new/ongoing collisions and collect entities ready to teleport
  for ( auto [actor_entity, actor_dir_cmp, actor_pos_cmp] : all_actors_view.each() )
  {
    for ( auto [wormhole_entity, wormhole_cmp, wh_pos_cmp] : wormhole_view.each() )
    {

      auto wh_hitbox_redux = Cmp::RectBounds( wh_pos_cmp.position, wh_pos_cmp.size, 1.f );
      if ( !actor_pos_cmp.findIntersection( wh_hitbox_redux.getBounds() ) ) continue;

      // Check if jump component already exists
      auto *wh_jump_cmp = getReg().try_get<Cmp::WormholeJump>( actor_entity );
      if ( !wh_jump_cmp )
      {
        // First collision - create component
        getReg().emplace<Cmp::WormholeJump>( actor_entity );
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
      auto [new_spawn_entity, new_spawn_pos_cmp] = get_random_position( IncludePack<Cmp::Obstacle>{},
                                                                        ExcludePack<Cmp::Wall, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC>{}, 0 );

      // destroy any obstacles at the new actor spawn position
      auto existing_obstacle_cmp = getReg().try_get<Cmp::Obstacle>( new_spawn_entity );
      if ( existing_obstacle_cmp )
      {
        SPDLOG_INFO( "Entity {} - Removing obstacle at new spawn ({}, {})", static_cast<uint32_t>( entity ), new_spawn_pos_cmp.position.x,
                     new_spawn_pos_cmp.position.y );
        existing_obstacle_cmp->m_integrity = 0.0f;
      }

      // update the teleported entity's components
      SPDLOG_INFO( "Entity {} - TELEPORTING NOW!", static_cast<uint32_t>( entity ) );
      getReg().remove<Cmp::LerpPosition>( entity );
      getReg().emplace_or_replace<Cmp::Position>( entity, new_spawn_pos_cmp.position, new_spawn_pos_cmp.size );
      getReg().remove<Cmp::WormholeJump>( entity );
      auto *npc_scan_bounds = getReg().try_get<Cmp::NPCScanBounds>( entity );
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
  auto wormhole_view = getReg().view<Cmp::WormholeSingularity>();
  for ( auto [entity, _] : wormhole_view.each() )
  {
    getReg().remove<Cmp::WormholeSingularity>( entity );
    SPDLOG_INFO( "Wormhole despawned (entity {})", static_cast<uint32_t>( entity ) );
  }

  auto wormhole_mb_view = getReg().view<Cmp::WormholeMultiBlock>();
  for ( auto [entity, _] : wormhole_mb_view.each() )
  {
    getReg().remove<Cmp::WormholeMultiBlock>( entity );
    getReg().remove<Cmp::SpriteAnimation>( entity );
    SPDLOG_INFO( "WormholeMultiBlock despawned (entity {})", static_cast<uint32_t>( entity ) );
  }
}

} // namespace ProceduralMaze::Sys