#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/Exit.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Persistent/ExitKeyRequirement.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/entity.hpp>

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Sys

{
ExitSystem::ExitSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                        entt::dispatcher &scenemanager_event_dispatcher )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
{
  SPDLOG_DEBUG( "ExitSystem initialized" );
}

void ExitSystem::spawn_exit( std::optional<sf::Vector2u> spawn_position )
{
  if ( spawn_position )
  {
    sf::FloatRect spawn_pos_px = sf::FloatRect( { static_cast<float>( spawn_position->x ) * Constants::kGridSquareSizePixels.x,
                                                  static_cast<float>( spawn_position->y ) * Constants::kGridSquareSizePixels.y },
                                                Constants::kGridSquareSizePixelsF );

    // remove any wall
    for ( auto [entt, wall_cmp, pos_cmp] : getReg().view<Cmp::Wall, Cmp::Position>().each() )
    {
      if ( spawn_pos_px.findIntersection( pos_cmp ) ) { getReg().remove<Cmp::Wall>( entt ); }
    }

    auto entity = getReg().create();
    getReg().emplace_or_replace<Cmp::Position>( entity, spawn_pos_px.position, Constants::kGridSquareSizePixelsF );
    getReg().emplace_or_replace<Cmp::Exit>( entity, true ); // locked at start
    getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "WALL", 1 );
    getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, spawn_pos_px.position.y );
    getReg().emplace_or_replace<Cmp::NpcNoPathFinding>( entity );

    SPDLOG_INFO( "Exit spawned at position ({}, {})", spawn_position->x, spawn_position->y );
    return;
  }
  else
  {
    auto [rand_entity, rand_pos_cmp] = Utils::Rnd::get_random_position(
        getReg(), {}, Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::Exit, Cmp::PlayerCharacter, Cmp::NPC, Cmp::ReservedPosition>{}, 0 );

    auto existing_obstacle_cmp = getReg().try_get<Cmp::Obstacle>( rand_entity );
    if ( existing_obstacle_cmp ) { getReg().remove<Cmp::Obstacle>( rand_entity ); }

    getReg().emplace_or_replace<Cmp::Exit>( rand_entity, true ); // locked at start
    getReg().emplace_or_replace<Cmp::SpriteAnimation>( rand_entity, 0, 0, true, "WALL", 0 );
    getReg().emplace_or_replace<Cmp::ZOrderValue>( rand_entity, rand_pos_cmp.position.y );
    getReg().emplace_or_replace<Cmp::NpcNoPathFinding>( rand_entity );
    SPDLOG_INFO( "Exit spawned at position ({}, {})", rand_pos_cmp.position.x, rand_pos_cmp.position.y );
  }
}

void ExitSystem::check_player_can_unlock_exit()
{

  auto exit_view = getReg().view<Cmp::Exit, Cmp::Position>();
  for ( auto [entity, exit_cmp, exit_pos_cmp] : exit_view.each() )
  {
    auto player_pos = Utils::get_player_position( getReg() );
    Cmp::RectBounds player_hitbox( player_pos.position, player_pos.size, 1.5f );
    auto [found_entt, found_carryitem_type] = Utils::get_player_inventory_type( getReg() );
    if ( player_hitbox.findIntersection( exit_pos_cmp ) and found_carryitem_type.contains( "exitkey" ) )
    {
      // otherwise unlock the exit
      auto exit_view = getReg().view<Cmp::Exit, Cmp::Position>();
      for ( auto [entity, exit_cmp, pos_cmp] : exit_view.each() )
      {
        exit_cmp.m_locked = false;
        getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "WALL", 1 );
        getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y - 16.f );
        if ( m_sound_bank.get_effect( "secret" ).getStatus() == sf::Sound::Status::Stopped ) m_sound_bank.get_effect( "secret" ).play();
        Factory::destroyInventory( getReg(), "CARRYITEM.exitkey" );
      }
    }
  }
}

void ExitSystem::check_exit_collision()
{
  auto exit_view = getReg().view<Cmp::Exit, Cmp::Position>();
  for ( auto [entity, exit_cmp, exit_pos_cmp] : exit_view.each() )
  {
    if ( exit_cmp.m_locked == true ) return;
    for ( auto [player_entity, pc_cmp, pc_pos_cmp] : getReg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
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