#include <Audio/SoundBank.hpp>
#include <Components/AnimData.hpp>
#include <Components/Exit.hpp>
#include <Components/Grave/GraveExitMultiBlock.hpp>
#include <Components/Grave/GraveExitSegment.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Persistent/ExitKeyRequirement.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

#include <SFML/System/Vector2.hpp>
#include <entt/entity/entity.hpp>
#include <stdexcept>

namespace Game::Sys

{
ExitSystem::ExitSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                        entt::dispatcher &scenemanager_event_dispatcher )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
{
  SPDLOG_DEBUG( "ExitSystem initialized" );
  std::ignore = get_systems_event_queue().sink<Events::PlayerActionEvent>().connect<&ExitSystem::on_player_action>( this );
}

void ExitSystem::spawn_exit()
{
  entt::entity selected_entity = entt::null;
  Cmp::Position selected_pos_cmp( { 0, 0 }, { 0, 0 } );
  const auto &kGraveExitSpritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.exit.locked" );
  const int kMaxAttempts = 100;

  int attempt_count = 0;
  while ( true )
  {
    auto exclude_list = Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::Exit, Cmp::PlayerCharacter, Cmp::NPC, Cmp::ReservedPosition>{};
    auto [rand_entity, rand_pos_cmp] = Utils::Rnd::get_random_position( reg(), {}, exclude_list, 0 );
    Cmp::Position multiblock_hitbox( rand_pos_cmp.position, kGraveExitSpritesheet.get_px_size() );

    bool collides_with_wall = false;
    for ( auto [wall_entt, wall_cmp, wall_pos_cmp] : reg().view<Cmp::Wall, Cmp::Position>().each() )
    {
      if ( multiblock_hitbox.findIntersection( wall_pos_cmp ) ) { collides_with_wall = true; }
    }
    if ( not collides_with_wall )
    {
      selected_entity = rand_entity;
      selected_pos_cmp = rand_pos_cmp;
      break;
    }

    attempt_count++;
    if ( attempt_count >= kMaxAttempts ) { throw std::runtime_error( "Unable to spawn graveyard exit" ); }
  }

  // Remove the existing wall obstacle first
  Factory::remove_obstacle( reg(), selected_entity, true );

  Factory::add_multiblock_with_segments<Cmp::GraveExitMultiBlock, Cmp::GraveExitSegment>( reg(), selected_pos_cmp.position, kGraveExitSpritesheet );

  // place the exit cmp at the bottom centre of the Cmp::GraveExitMultiBlock
  sf::Vector2f door_grid_pos( ( kGraveExitSpritesheet.get_grid_size().x / 2 ), kGraveExitSpritesheet.get_grid_size().y - 2 );

  auto exit_entt = reg().create();
  auto door_px_pos = sf::Vector2f( selected_pos_cmp.x() + ( door_grid_pos.x * Constants::kGridSizePxF.x ),
                                   selected_pos_cmp.y() + ( door_grid_pos.y * Constants::kGridSizePxF.y ) );
  reg().emplace_or_replace<Cmp::Position>( exit_entt, door_px_pos, Constants::kGridSizePxF );
  reg().emplace_or_replace<Cmp::Exit>( exit_entt, true );
  SPDLOG_INFO( "Exit spawned at position ({}, {})", selected_pos_cmp.position.x, selected_pos_cmp.position.y );
}

void ExitSystem::on_player_action( Events::PlayerActionEvent ev )
{
  auto [entt, inventory_ms] = Utils::Player::get_inventory_type( reg() );
  if ( ev.action == Events::PlayerActionEvent::GameActions::ACTIVATE && inventory_ms == "sprite.item.exitkey" ) { check_player_can_unlock_exit(); }
}

void ExitSystem::check_player_can_unlock_exit()
{
  auto [inv_entt, inv_type] = Utils::Player::get_inventory_type( reg() );
  if ( not inv_type.contains( "exitkey" ) ) return;

  auto player_pos = Utils::Player::get_position( reg() );
  auto player_hitbox = Cmp::RectBounds::scaled( player_pos, 2.f );

  // assume player should be behind the exit multiblock
  for ( auto [exit_mb_entt, exit_mb_cmp, anim_cmp] : reg().view<Cmp::GraveExitMultiBlock, Cmp::AnimData>().each() )
  {
    // const auto &grave_exit_ss = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.exit.unlocked" );
    reg().emplace_or_replace<Cmp::ZOrderValue>( exit_mb_entt, exit_mb_cmp.position.y + ( 5 * Constants::kGridSizePxF.y ) );
  }

  for ( auto [entity, exit_cmp, exit_pos_cmp] : reg().view<Cmp::Exit, Cmp::Position>().each() )
  {
    if ( not player_hitbox.findIntersection( exit_pos_cmp ) ) continue;

    exit_cmp.m_locked = false;

    for ( auto [exit_mb_entt, exit_mb_cmp, anim_cmp] : reg().view<Cmp::GraveExitMultiBlock, Cmp::AnimData>().each() )
    {
      if ( not exit_pos_cmp.findIntersection( exit_mb_cmp ) ) continue;
      anim_cmp.m_sprite_type = "sprite.graveyard.exit.unlocked";
      // if we are near exit then place exit multiblock behind the player
      reg().emplace_or_replace<Cmp::ZOrderValue>( exit_mb_entt, exit_mb_cmp.position.y - 16.f );

      // remove all blocking so player can exit
      for ( auto [entity, pos_cmp] : reg().view<Cmp::Position>().each() )
      {
        if ( not exit_pos_cmp.findIntersection( pos_cmp ) ) continue;
        reg().remove<Cmp::PlayerNoPath>( entity );
      }
      break;
    }

    if ( m_sound_bank.get_effect( "secret" ).getStatus() == sf::Sound::Status::Stopped ) m_sound_bank.get_effect( "secret" ).play();
    Factory::destroy_inventory( reg(), "sprite.item.exitkey" );
  }
}

void ExitSystem::check_exit_collision()
{
  auto exit_view = reg().view<Cmp::Exit, Cmp::Position>();
  for ( auto [entity, exit_cmp, exit_pos_cmp] : exit_view.each() )
  {
    if ( exit_cmp.m_locked ) { return; }

    auto pc_pos_cmp = Utils::Player::get_position( reg() );
    if ( pc_pos_cmp.findIntersection( exit_pos_cmp ) )
    {
      SPDLOG_INFO( "Player reached the exit zone!" );
      Utils::get_system_cmp( reg() ).level_complete = true;
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::LEVEL_COMPLETE );
      Factory::remove_player_last_graveyard_pos( reg() );
    }
  }
}

} // namespace Game::Sys