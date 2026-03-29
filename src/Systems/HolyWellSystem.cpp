#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <Audio/SoundBank.hpp>
#include <Components/Exit.hpp>
#include <Components/HolyWell/HolyWellEntrance.hpp>
#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/HolyWell/HolyWellSegment.hpp>
#include <Components/Inventory/FlashUIWealth.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Wall.hpp>
#include <Events/DropInventoryEvent.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Player/PlayerWealth.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/HolyWellSystem.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Sys
{

void HolyWellSystem::spawn_well( sf::Vector2u spawn_position )
{

  const Sprites::MultiSprite &ms = m_sprite_factory.get_multisprite_by_type( "HOLYWELL.interior_well" );
  const sf::Vector2f new_pos = { static_cast<float>( spawn_position.x ) * Constants::kGridSizePx.x,
                                 static_cast<float>( spawn_position.y ) * Constants::kGridSizePx.y };
  Factory::add_multiblock_with_segments<Cmp::HolyWellMultiBlock, Cmp::HolyWellSegment>( getReg(), new_pos, ms );
}

void HolyWellSystem::on_player_action( Events::PlayerActionEvent ev )
{
  if ( ev.action != Events::PlayerActionEvent::GameActions::ACTIVATE ) return;
  check_inventory_deposit();
}

void HolyWellSystem::check_entrance_collision()
{
  auto pc_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto holywelldoor_view = getReg().view<Cmp::HollyWellEntrance, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, holywelldoor_cmp, holywell_door_pos_cmp] : holywelldoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), holywell_door_pos_cmp ) ) continue;

      // shrink entrance bounds slightly for better UX
      Cmp::RectBounds decreased_entrance_bounds( holywell_door_pos_cmp.position, holywell_door_pos_cmp.size, 0.1f,
                                                 Cmp::RectBounds::ScaleCardinality::BOTH );

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      SPDLOG_INFO( "check_entrance_collision: Player entering holywell from graveyard at position ({}, {})", pc_pos_cmp.position.x,
                   pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_HOLYWELL );

      Factory::add_player_last_graveyard_pos( getReg(), Utils::Player::get_position( getReg() ), { 0.f, 0.f } );
    }
  }
}

void HolyWellSystem::check_exit_collision()
{
  auto pc_view = getReg().view<Cmp::PlayerCharacter, Cmp::Position>();
  auto holywelldoor_view = getReg().view<Cmp::Exit, Cmp::Position>();

  for ( auto [pc_entity, pc_cmp, pc_pos_cmp] : pc_view.each() )
  {
    for ( auto [door_entity, holywelldoor_cmp, holywell_door_pos_cmp] : holywelldoor_view.each() )
    {
      // optimize: skip if not visible
      if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), holywell_door_pos_cmp ) ) continue;

      Cmp::RectBounds decreased_entrance_bounds( holywell_door_pos_cmp.position, holywell_door_pos_cmp.size, 0.1f,
                                                 Cmp::RectBounds::ScaleCardinality::BOTH ); // shrink entrance bounds slightly for better UX

      if ( not pc_pos_cmp.findIntersection( decreased_entrance_bounds.getBounds() ) ) continue;

      SPDLOG_INFO( "check_exit_collision: Player exiting holywell to graveyard at position ({}, {})", pc_pos_cmp.position.x, pc_pos_cmp.position.y );
      m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_HOLYWELL );
    }
  }
}

void HolyWellSystem::check_inventory_deposit()
{

  auto [inventory_entt, inventory_type] = Utils::Player::get_inventory_type( getReg() );
  if ( not inventory_type.contains( "CARRYITEM.jewelry" ) ) return;

  auto &wealth = Utils::Player::get_wealth( m_reg );

  // check if we're near a holywell
  auto player_hitbox = Cmp::RectBounds( Utils::Player::get_position( getReg() ).position, Constants::kGridSizePxF, 1.5f );
  for ( auto [well_entt, well_mb_cmp] : getReg().view<Cmp::HolyWellMultiBlock>().each() )
  {
    if ( not player_hitbox.findIntersection( well_mb_cmp ) ) continue;
    Factory::destroy_inventory( getReg(), inventory_type );
    wealth.wealth += 2;
    m_sound_bank.get_effect( "get_key" ).play();

    // signal UI to flash
    auto flash_entt = getReg().create();
    getReg().emplace_or_replace<Cmp::FlashUIWealth>( flash_entt );
  }
  m_inventory_deposit_interval = sf::Time::Zero;
}

} // namespace ProceduralMaze::Sys