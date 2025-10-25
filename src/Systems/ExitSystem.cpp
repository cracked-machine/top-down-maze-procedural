#include <Components/Exit.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/System.hpp>
#include <Door.hpp>
#include <NPC.hpp>
#include <Persistent/PlayerStartPosition.hpp>
#include <Random.hpp>
#include <RenderSystem.hpp>
#include <ReservedPosition.hpp>
#include <SFML/System/Vector2.hpp>
#include <Systems/ExitSystem.hpp>
#include <Wall.hpp>

namespace ProceduralMaze::Sys {

ExitSystem::ExitSystem( ProceduralMaze::SharedEnttRegistry reg )
    : BaseSystem( reg )
{
  // register the event sinks
  std::ignore = getEventDispatcher().sink<Events::UnlockDoorEvent>().connect<&ExitSystem::on_door_unlock_event>( this );
}

void ExitSystem::spawn_exit()
{
  auto [rand_entity, rand_pos_cmp] = get_random_position(
      {}, ExcludePack<Cmp::Wall, Cmp::Door, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC, Cmp::ReservedPosition>{}, 0 );

  auto existing_obstacle_cmp = m_reg->try_get<Cmp::Obstacle>( rand_entity );
  if ( existing_obstacle_cmp ) { existing_obstacle_cmp->m_integrity = 0.0f; }

  m_reg->emplace_or_replace<Cmp::Door>( rand_entity, "WALL", 0 );
  m_reg->emplace_or_replace<Cmp::Exit>( rand_entity, true ); // locked at start

  SPDLOG_INFO( "Spawned exit at position ({}, {})", rand_pos_cmp.position.x, rand_pos_cmp.position.y );
}

void ExitSystem::unlock_exit()
{
  auto exit_view = m_reg->view<Cmp::Exit, Cmp::Door>();
  for ( auto [entity, exit_cmp, door_cmp] : exit_view.each() )
  {
    exit_cmp.m_locked = false;
    door_cmp.m_tile_index = 1; // open door tile
  }
}

void ExitSystem::check_exit_collision()
{
  auto exit_view = m_reg->view<Cmp::Exit, Cmp::Position>();
  for ( auto [entity, exit_cmp, exit_pos_cmp] : exit_view.each() )
  {
    if ( exit_cmp.m_locked ) return;
    for ( auto [player_entity, pc_cmp, pc_pos_cmp] : m_reg->view<Cmp::PlayableCharacter, Cmp::Position>().each() )
    {
      if ( pc_pos_cmp.findIntersection( exit_pos_cmp ) )
      {
        SPDLOG_DEBUG( "Player reached the exit zone!" );
        for ( auto [_entt, _sys] : m_reg->view<Cmp::System>().each() )
        {
          _sys.level_complete = true;
        }
      }
    }
  }
}

} // namespace ProceduralMaze::Sys