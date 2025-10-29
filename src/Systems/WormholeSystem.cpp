#include <Door.hpp>
#include <LerpPosition.hpp>
#include <NPC.hpp>
#include <Obstacle.hpp>
#include <Persistent/WormholeSeed.hpp>
#include <PlayableCharacter.hpp>
#include <ReservedPosition.hpp>
#include <SFML/System/Vector2.hpp>
#include <SpriteAnimation.hpp>
#include <Systems/RenderSystem.hpp>
#include <Wall.hpp>
#include <Wormhole.hpp>
#include <WormholeSystem.hpp>

namespace ProceduralMaze::Sys {

WormholeSystem::WormholeSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window )
    : BaseSystem( reg, window )
{
  init_context();
  SPDLOG_DEBUG( "WormholeSystem initialized" );
}

void WormholeSystem::init_context()
{
  // ensure we have a persistent component for the wormhole seed
  add_persistent_component<Cmp::Persistent::WormholeSeed>();
}

void WormholeSystem::spawn_wormhole( SpawnPhase phase )
{
  // 1. pick a random position component in the maze, exclude walls, doors, exits, and playable characters
  // 2. get the entity at that position
  unsigned long seed = 0;
  if ( phase == SpawnPhase::InitialSpawn ) seed = get_persistent_component<Cmp::Persistent::WormholeSeed>().get_value();
  auto [random_entity, random_pos] = get_random_position(
      IncludePack<Cmp::Obstacle>{},
      ExcludePack<Cmp::Wall, Cmp::Door, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC, Cmp::ReservedPosition>{}, seed );

  // 3. set the entities obstacle component to "broken" so we have something for the shader effect to mangle
  auto obstacle_cmp = m_reg->try_get<Cmp::Obstacle>( random_entity );
  if ( obstacle_cmp )
  {
    obstacle_cmp->m_enabled = false;

    for ( int i = -1; i < 2; ++i )
    {
      for ( int j = -1; j < 2; ++j )
      {
        sf::Vector2f offset = { static_cast<float>( i ) * BaseSystem::kGridSquareSizePixels.x,
                                static_cast<float>( j ) * BaseSystem::kGridSquareSizePixels.y };

        // Calculate the adjacent position
        sf::Vector2f adjacent_position = random_pos.position + offset;

        // Find entity at this adjacent position
        auto position_view = m_reg->view<Cmp::Position, Cmp::Obstacle>();
        for ( auto [entity, pos_cmp, adj_obstacle_cmp] : position_view.each() )
        {
          // Check if this entity is at the adjacent position we're looking for
          if ( pos_cmp.position.x == adjacent_position.x && pos_cmp.position.y == adjacent_position.y )
          {
            // Found the entity at the adjacent position
            // Do whatever you need with this entity
            adj_obstacle_cmp.m_enabled = false;
            SPDLOG_DEBUG( "Found adjacent entity {} at position ({}, {})", static_cast<uint32_t>( entity ), pos_cmp.position.x,
                          pos_cmp.position.y );
            break; // Move to next offset
          }
        }
      }
    }
  }

  // 4. add the wormhole component to the entity
  m_reg->emplace_or_replace<Cmp::Wormhole>( random_entity );
  m_reg->emplace_or_replace<Cmp::SpriteAnimation>( random_entity );

  SPDLOG_INFO( "Wormhole spawned at position ({}, {})", random_pos.position.x, random_pos.position.y );
}

void WormholeSystem::check_player_wormhole_collision()
{
  // 1. iterate wormhole and player view,
  auto wormhole_view = m_reg->view<Cmp::Wormhole, Cmp::Position>();
  auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();
  for ( auto [player_entity, player_cmp, player_pos_cmp] : player_view.each() )
  {
    for ( auto [wormhole_entity, wormhole_cmp, wh_pos_cmp] : wormhole_view.each() )
    {
      if ( !is_visible_in_view( RenderSystem::getGameView(), wh_pos_cmp ) ) continue;

      // 2. check for collision,
      if ( !player_pos_cmp.findIntersection( wh_pos_cmp ) ) continue;
      SPDLOG_INFO( "Player collided with wormhole at position ({}, {})", wh_pos_cmp.position.x, wh_pos_cmp.position.y );

      // 3. if collision, pick a random new player spawn location. Exclude walls, doors, exits, playable characters and
      // NPCs
      auto [new_spawn_entity, new_spawn_pos_cmp] = get_random_position(
          IncludePack<Cmp::Obstacle>{}, ExcludePack<Cmp::Wall, Cmp::Door, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC>{}, 0 );

      // 6. call despawn_wormhole()
      despawn_wormhole();

      // 4. remove the obstacle from the new spawn location
      auto existing_obstacle_cmp = m_reg->try_get<Cmp::Obstacle>( new_spawn_entity );
      if ( existing_obstacle_cmp )
      {
        SPDLOG_INFO( "Removing obstacle at new spawn position ({}, {})", new_spawn_pos_cmp.position.x,
                     new_spawn_pos_cmp.position.y );
        existing_obstacle_cmp->m_integrity = 0.0f;
      }

      // 5. teleport player to the location, abort lerp if active
      m_reg->remove<Cmp::LerpPosition>( player_entity );
      m_reg->emplace_or_replace<Cmp::Position>( player_entity, new_spawn_pos_cmp.position, new_spawn_pos_cmp.size );
      SPDLOG_INFO( "Player teleported to new position ({}, {})", new_spawn_pos_cmp.position.x, new_spawn_pos_cmp.position.y );

      // 7. call spawn_wormhole()
      spawn_wormhole( WormholeSystem::SpawnPhase::Respawn );

      // 8. Exit immediately after teleporting to prevent multiple teleports in the same frame
      return;
    }
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