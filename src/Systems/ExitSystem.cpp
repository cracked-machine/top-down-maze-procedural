#include <Audio/SoundBank.hpp>
#include <Components/AnimData.hpp>
#include <Components/Exit.hpp>
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
#include <Constants.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Systems/ExitSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

#include <SFML/System/Vector2.hpp>
#include <entt/entity/entity.hpp>

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

  auto [rand_entity, rand_pos_cmp] = Utils::Rnd::get_random_position(
      reg(), {}, Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::Exit, Cmp::PlayerCharacter, Cmp::NPC, Cmp::ReservedPosition>{}, 0 );

  // Remove the existing wall obstacle first
  Factory::remove_obstacle( reg(), rand_entity, true );

  const auto &ss_main = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.exit.main" );
  const auto &ss_cap = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.exit.cap" );
  auto uuid = Cmp::UUID::generate();

  Factory::add_obstacle( reg(), rand_entity );
  reg().emplace_or_replace<Cmp::Exit>( rand_entity, true );
  Factory::decorate_obstacle( reg(), rand_entity, rand_pos_cmp, ss_main, 0 );
  reg().emplace_or_replace<Cmp::UUID>( rand_entity, uuid );

  auto cap_entt = reg().create();
  Cmp::Position cap_position( { rand_pos_cmp.x(), rand_pos_cmp.y() - Constants::kGridSizePxF.y }, Constants::kGridSizePxF );
  reg().emplace_or_replace<Cmp::Position>( cap_entt, cap_position );
  Factory::decorate_obstacle( reg(), cap_entt, cap_position, ss_cap, 0, rand_pos_cmp.y(), false );
  reg().emplace_or_replace<Cmp::UUID>( cap_entt, uuid );
  reg().emplace_or_replace<Cmp::ReservedPosition>( cap_entt );

  reg().emplace_or_replace<Cmp::ReservedPosition>( rand_entity );

  SPDLOG_INFO( "Exit spawned at position ({}, {})", rand_pos_cmp.position.x, rand_pos_cmp.position.y );
}

void ExitSystem::on_player_action( Events::PlayerActionEvent ev )
{
  auto [entt, inventory_ms] = Utils::Player::get_inventory_type( reg() );
  if ( ev.action == Events::PlayerActionEvent::GameActions::ACTIVATE && inventory_ms == "sprite.item.exitkey" ) { check_player_can_unlock_exit(); }
}

void ExitSystem::check_player_can_unlock_exit()
{

  auto exit_view = reg().view<Cmp::Exit, Cmp::Position>();
  for ( auto [entity, exit_cmp, exit_pos_cmp] : exit_view.each() )
  {
    auto player_pos = Utils::Player::get_position( reg() );
    auto player_hitbox = Cmp::RectBounds::scaled( player_pos, 5.f );
    auto [found_entt, found_carryitem_type] = Utils::Player::get_inventory_type( reg() );
    if ( player_hitbox.findIntersection( exit_pos_cmp ) and found_carryitem_type.contains( "exitkey" ) )
    {
      exit_cmp.m_locked = false;
      Factory::remove_obstacle( reg(), entity, true );

      // clang-format off
      reg().emplace_or_replace<Cmp::AnimData>( entity, Cmp::AnimData::Config{ 
            .sprite_type = "sprite.graveyard.exit.unlocked",
            .enabled = true
      });
      // clang-format on

      reg().emplace_or_replace<Cmp::ZOrderValue>( entity, exit_pos_cmp.position.y );
      reg().remove<Cmp::PlayerNoPath>( entity );
      if ( m_sound_bank.get_effect( "secret" ).getStatus() == sf::Sound::Status::Stopped ) m_sound_bank.get_effect( "secret" ).play();
      Factory::destroy_inventory( reg(), "sprite.item.exitkey" );
    }
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
      Utils::getSystemCmp( reg() ).level_complete = true;
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::LEVEL_COMPLETE );
      Factory::remove_player_last_graveyard_pos( reg() );
    }
  }
}

} // namespace Game::Sys