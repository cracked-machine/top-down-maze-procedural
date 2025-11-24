#include <Components/PlayerKeysCount.hpp>
#include <Events/SceneManagerEvent.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Door.hpp>
#include <Components/Exit.hpp>
#include <Components/NPC.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Random.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
{

ExitSystem::ExitSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                        Audio::SoundBank &sound_bank, entt::dispatcher &scenemanager_event_dispatcher )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
{
  // The entt::dispatcher is independent of the registry, so it is safe to bind event handlers in the constructor
  std::ignore = get_systems_event_queue().sink<Events::UnlockDoorEvent>().connect<&ExitSystem::on_door_unlock_event>( this );
  SPDLOG_DEBUG( "ExitSystem initialized" );
}

void ExitSystem::spawn_exit()
{
  auto [rand_entity, rand_pos_cmp] = get_random_position(
      {}, ExcludePack<Cmp::Wall, Cmp::Door, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC, Cmp::ReservedPosition>{}, 0 );

  auto existing_obstacle_cmp = getReg().try_get<Cmp::Obstacle>( rand_entity );
  if ( existing_obstacle_cmp ) { existing_obstacle_cmp->m_integrity = 0.0f; }

  getReg().emplace_or_replace<Cmp::Door>( rand_entity, "WALL", 0 );
  getReg().emplace_or_replace<Cmp::Exit>( rand_entity, true ); // locked at start

  SPDLOG_INFO( "Spawned exit at position ({}, {})", rand_pos_cmp.position.x, rand_pos_cmp.position.y );
}

void ExitSystem::unlock_exit()
{

  auto player_key_view = getReg().view<Cmp::PlayerKeysCount>();
  for ( auto [pk_entity, pk_cmp] : player_key_view.each() )
  {
    if ( pk_cmp.get_count() < get_persistent_component<Cmp::Persistent::MaxNumAltars>().get_value() )
    {
      SPDLOG_DEBUG( "Not enough keys to unlock exit ({} / {})", pk_cmp.get_count(),
                    get_persistent_component<Cmp::Persistent::MaxNumAltars>().get_value() );
      return;
    }
  }

  // otherwise unlock the exit
  auto exit_view = getReg().view<Cmp::Exit, Cmp::Door>();
  for ( auto [entity, exit_cmp, door_cmp] : exit_view.each() )
  {
    exit_cmp.m_locked = false;
    door_cmp.m_tile_index = 1; // open door tile
    m_sound_bank.get_effect( "secret" ).play();
  }
}

void ExitSystem::check_exit_collision()
{
  auto exit_view = getReg().view<Cmp::Exit, Cmp::Position>();
  for ( auto [entity, exit_cmp, exit_pos_cmp] : exit_view.each() )
  {
    auto max_num_shrines = get_persistent_component<Cmp::Persistent::MaxNumAltars>();
    if ( exit_cmp.m_locked == true ) return;
    for ( auto [player_entity, pc_cmp, pc_pos_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::Position>().each() )
    {
      if ( pc_pos_cmp.findIntersection( exit_pos_cmp ) )
      {
        SPDLOG_DEBUG( "Player reached the exit zone!" );
        for ( auto [_entt, _sys] : getReg().view<Cmp::System>().each() )
        {
          _sys.level_complete = true;
          m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::LEVEL_COMPLETE );
        }
      }
    }
  }
}

} // namespace ProceduralMaze::Sys