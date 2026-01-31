#include <Audio/SoundBank.hpp>
#include <Components/Player/PlayerRuinLocation.hpp>
#include <Components/RectBounds.hpp>
#include <Components/Ruin/RuinEntrance.hpp>
#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Components/Ruin/RuinMultiBlock.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <Components/Ruin/RuinStairsSegment.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <SceneControl/Events/SceneManagerEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/RuinSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

RuinSystem::RuinSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
                        entt::dispatcher &scenemanager_event_dispatcher )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_scenemanager_event_dispatcher( scenemanager_event_dispatcher )
{
  SPDLOG_DEBUG( "PlayerSystem initialized" );
}

void RuinSystem::update()
{
  auto player_pos = Utils::get_player_position( getReg() );
  auto ruindoor_view = getReg().view<Cmp::RuinEntrance, Cmp::Position>();
  for ( auto [door_entity, ruindoor_cmp, door_pos_cmp] : ruindoor_view.each() )
  {
    // optimize: skip if not visible
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), door_pos_cmp ) ) continue;

    // Player can't intersect with a closed crypt door so expand their hitbox to facilitate collision detection
    auto player_hitbox = Cmp::RectBounds( player_pos.position, player_pos.size, 0.5f );
    if ( not player_hitbox.findIntersection( door_pos_cmp ) ) continue;

    auto ruin_mb_view = getReg().view<Cmp::RuinMultiBlock>();
    for ( auto [ruin_mb_entity, ruin_mb_cmp] : ruin_mb_view.each() )
    {
      if ( door_pos_cmp.findIntersection( ruin_mb_cmp ) )
      {
        getReg().emplace_or_replace<Cmp::ZOrderValue>( ruin_mb_entity, player_pos.position.y - 16.f );
        SPDLOG_INFO( "check_entrance_collision: Player entering ruin from graveyard at position ({}, {})", player_pos.position.x,
                     player_pos.position.y );
        m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_RUIN_LOWER );

        // remember player position
        auto last_player_pos = Factory::add_player_last_graveyard_pos( getReg(), door_pos_cmp );

        // drop any inventory outside the door
        auto [inventory_entt, inventory_slot_type] = Utils::get_player_inventory_type( getReg() );
        auto dropped_entt = Factory::dropInventorySlotIntoWorld( getReg(), last_player_pos,
                                                                 m_sprite_factory.get_multisprite_by_type( inventory_slot_type ), inventory_entt );
        if ( dropped_entt != entt::null ) { m_sound_bank.get_effect( "drop_relic" ).play(); }

        auto player_entt = Utils::get_player_entity( getReg() );
        getReg().emplace_or_replace<Cmp::PlayerRuinLocation>( player_entt, Cmp::PlayerRuinLocation::Floor::LOWER );
      }
      else { getReg().emplace_or_replace<Cmp::ZOrderValue>( ruin_mb_entity, player_pos.position.y + 16.f ); }
    }
  }
}

// void RuinSystem::spawn_objective( sf::Vector2f spawn_position, Sprites::SpriteMetaType type )
// {

//   Factory::createCarryItem( getReg(), Cmp::Position( spawn_position, Constants::kGridSquareSizePixelsF ), type );
// }

void RuinSystem::spawn_floor_access( sf::Vector2f spawn_position, sf::Vector2f size, Cmp::RuinFloorAccess::Direction dir )
{
  auto floor_access_entt = getReg().create();
  getReg().emplace_or_replace<Cmp::RuinFloorAccess>( floor_access_entt, spawn_position, size, dir );
  SPDLOG_INFO( "Spawning floor access at {},{}", spawn_position.x, spawn_position.y );
}

void RuinSystem::spawn_staircase( sf::Vector2f spawn_position, const Sprites::MultiSprite &stairs_ms )
{
  auto stairs_entt = getReg().create();
  Cmp::Position stairs_pos( spawn_position, stairs_ms.getSpriteSizePixels() );
  getReg().emplace_or_replace<Cmp::Position>( stairs_entt, spawn_position, stairs_ms.getSpriteSizePixels() );
  Factory::createMultiblock<Cmp::RuinMultiBlock>( getReg(), stairs_entt, stairs_pos, stairs_ms );
  Factory::createMultiblockSegments<Cmp::RuinMultiBlock, Cmp::RuinStairsSegment>( getReg(), stairs_entt, stairs_pos, stairs_ms );

  for ( auto [stairs_entt, stairs_cmp, stairs_zorder] : getReg().view<Cmp::RuinMultiBlock, Cmp::ZOrderValue>().each() )
  {
    stairs_zorder.setZOrder( 0 );
  }
}

void RuinSystem::check_floor_access_collision( Cmp::RuinFloorAccess::Direction direction )
{
  if ( m_floor_access_cooldown.getElapsedTime().asSeconds() < kFloorAccessCooldownSeconds ) { return; }

  auto player_pos = Utils::get_player_position( getReg() );
  bool currently_on_floor_access = false;

  for ( auto [access_entt, access_cmp] : getReg().view<Cmp::RuinFloorAccess>().each() )
  {
    if ( player_pos.findIntersection( access_cmp ) )
    {
      currently_on_floor_access = true;

      // Only trigger if player wasn't already on floor access (must leave and re-enter)
      if ( m_was_on_floor_access ) { continue; }

      // Check if THIS floor access entity leads in the direction we're looking for
      if ( access_cmp.m_direction != direction ) { continue; }

      switch ( access_cmp.m_direction )
      {
        case Cmp::RuinFloorAccess::Direction::TO_UPPER:
          m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::ENTER_RUIN_UPPER );
          break;
        case Cmp::RuinFloorAccess::Direction::TO_LOWER:
          m_scenemanager_event_dispatcher.enqueue<Events::SceneManagerEvent>( Events::SceneManagerEvent::Type::EXIT_RUIN_UPPER );
          break;
      }
    }
  }

  // Update tracking - player must leave floor access area before it can trigger again
  m_was_on_floor_access = currently_on_floor_access;
}

} // namespace ProceduralMaze::Sys