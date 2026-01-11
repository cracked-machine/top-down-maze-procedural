#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteOffset.hpp>
#include <Components/AbsoluteRotation.hpp>

#include <Components/DeathPosition.hpp>
#include <Components/Direction.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/InventoryWearLevel.hpp>
#include <Components/Neighbours.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/PCDetectionBounds.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCadaverCount.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/PlayerWealth.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SFML/System/Vector2.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/fwd.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Factory
{

void CreatePlayer( entt::registry &registry )
{
  SPDLOG_INFO( "Creating player entity" );
  auto entity = registry.create();

  // start position must be pixel coordinates within the screen resolution (kDisplaySize),
  // but also grid aligned (kMapGridSize) to avoid collision detection errors.
  // So we must recalc start position to the nearest grid position here
  auto start_pos = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerStartPosition>( registry );
  start_pos = Utils::snap_to_grid( start_pos );
  registry.emplace<Cmp::Position>( entity, start_pos, Constants::kGridSquareSizePixelsF );

  auto &blast_radius = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::BlastRadius>( registry );
  registry.emplace<Cmp::PlayableCharacter>( entity, blast_radius.get_value() );

  registry.emplace<Cmp::Direction>( entity, sf::Vector2f{ 0, 0 } );

  auto &pc_detection_scale = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerDetectionScale>( registry );

  registry.emplace<Cmp::PCDetectionBounds>( entity, start_pos, Constants::kGridSquareSizePixelsF, pc_detection_scale.get_value() );

  registry.emplace<Cmp::SpriteAnimation>( entity, 0, 0, true, "PLAYER.walk.south" );
  registry.emplace<Cmp::PlayerRelicCount>( entity, 0 );
  registry.emplace<Cmp::PlayerCadaverCount>( entity, 0 );
  registry.emplace<Cmp::PlayerHealth>( entity, 100 );
  registry.emplace<Cmp::PlayerWealth>( entity, 0 );
  registry.emplace<Cmp::PlayerMortality>( entity, Cmp::PlayerMortality::State::ALIVE );

  registry.emplace<Cmp::ZOrderValue>( entity, start_pos.y ); // z-order based on y-position
  registry.emplace<Cmp::AbsoluteAlpha>( entity, 255 );       // fully opaque
  registry.emplace<Cmp::AbsoluteRotation>( entity, 0 );

  auto inventory_entity = registry.create();
  registry.emplace<Cmp::PlayerInventorySlot>( inventory_entity, "CARRYITEM.pickaxe" );
  registry.emplace_or_replace<Cmp::InventoryWearLevel>( inventory_entity, 100.f );
  registry.emplace<Cmp::SpriteAnimation>( inventory_entity, 0, 0, true, "CARRYITEM.pickaxe", 0 );
}

entt::entity createWorldPosition( entt::registry &registry, const sf::Vector2f &pos )
{
  auto entity = registry.create();
  registry.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSquareSizePixelsF );
  registry.emplace_or_replace<Cmp::Neighbours>( entity );
  return entity;
}

entt::entity createVoidPosition( entt::registry &registry, const sf::Vector2f &pos )
{
  auto entity = registry.create();
  registry.emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSquareSizePixelsF );
  registry.emplace_or_replace<Cmp::ReservedPosition>( entity );
  return entity;
}

void addSpawnArea( entt::registry &registry, entt::entity entity, float zorder )
{
  // We need to reserve these positions for the player start area
  registry.emplace_or_replace<Cmp::ReservedPosition>( entity );
  registry.emplace_or_replace<Cmp::SpawnArea>( entity, false );
  registry.emplace_or_replace<Cmp::NoPathFinding>( entity );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "PLAYERSPAWN", 0 );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entity, zorder );
}

void createPlayerDeathAnim( entt::registry &registry, Cmp::Position player_pos_cmp, const Sprites::MultiSprite &sprite )
{
  auto player_blood_splat_entity = registry.create();
  sf::Vector2f offset;
  if ( ( sprite.getSpriteSizePixels().x == Constants::kGridSquareSizePixelsF.x ) and
       ( sprite.getSpriteSizePixels().y == Constants::kGridSquareSizePixelsF.y ) )
  {
    offset = sf::Vector2f{ 0, 0 };
  }
  else { offset = sprite.getSpriteSizePixels() / 2.f; }
  registry.emplace<Cmp::Position>( player_blood_splat_entity, player_pos_cmp.position - offset, player_pos_cmp.size );
  registry.emplace_or_replace<Cmp::DeathPosition>( player_blood_splat_entity, player_pos_cmp.position - offset, player_pos_cmp.size );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( player_blood_splat_entity, 0, 0, true, sprite.get_sprite_type(), 0 );
  registry.emplace_or_replace<Cmp::ZOrderValue>( player_blood_splat_entity, player_pos_cmp.position.y * 3 ); // always infront
}

//! @brief Create a Carry Item object in the world
//! @param reg the ECS registry
//! @param pos the position to place the new item
//! @param type the item type. See "CARRYITEM.xxxx" in res/json/sprite_metadata.json
//! @return entt::entity
entt::entity createCarryItem( entt::registry &reg, Cmp::Position pos, Sprites::SpriteMetaType type, float zorder )
{
  auto world_carry_item_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( world_carry_item_entt, pos.position, pos.size );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( world_carry_item_entt, 0, 0, true, type, 0 );
  reg.emplace_or_replace<Cmp::ZOrderValue>( world_carry_item_entt, pos.position.y - 1.f + zorder );
  reg.emplace_or_replace<Cmp::CarryItem>( world_carry_item_entt, type );
  reg.emplace_or_replace<Cmp::NoPathFinding>( world_carry_item_entt );
  if ( type == "CARRYITEM.axe" || type == "CARRYITEM.pickaxe" || type == "CARRYITEM.shovel" )
  {
    reg.emplace_or_replace<Cmp::InventoryWearLevel>( world_carry_item_entt, 100.f );
  }

  return world_carry_item_entt;
}

//! @brief Remove the CarryItem from player inventory and place it into the world
//! @param reg the ECS registry
//! @param pos the postion to place the item
//! @param sprite the multisprite object
//! @param inventory_slot_cmp_entt the player inventory slot entt
//! @return entt::entity
entt::entity dropCarryItem( entt::registry &reg, Cmp::Position pos, const Sprites::MultiSprite &sprite, entt::entity inventory_slot_cmp_entt )
{
  auto inventory_slot_cmp = reg.try_get<Cmp::PlayerInventorySlot>( inventory_slot_cmp_entt );
  auto inventory_slot_level_cmp = reg.try_get<Cmp::InventoryWearLevel>( inventory_slot_cmp_entt );

  if ( not inventory_slot_cmp ) return entt::null;

  // if plant then replant it in the ground
  if ( inventory_slot_cmp->type.contains( "plant" ) )
  {
    auto world_carry_item_entt = Factory::createPlantObstacle( reg, pos, inventory_slot_cmp->type, 0.f );
    reg.destroy( inventory_slot_cmp_entt );
    return world_carry_item_entt;
  }
  else
  {
    // otherwise just drop it as a Re-pickupable item
    auto world_carry_item_entt = reg.create();
    reg.emplace_or_replace<Cmp::Position>( world_carry_item_entt, pos.position, pos.size );
    reg.emplace_or_replace<Cmp::SpriteAnimation>( world_carry_item_entt, 0, 0, false, sprite.get_sprite_type(), 0 );
    reg.emplace_or_replace<Cmp::ZOrderValue>( world_carry_item_entt, pos.position.y - 1.f );
    reg.emplace_or_replace<Cmp::CarryItem>( world_carry_item_entt, inventory_slot_cmp->type );
    reg.emplace_or_replace<Cmp::NoPathFinding>( world_carry_item_entt );

    if ( inventory_slot_level_cmp ) { reg.emplace_or_replace<Cmp::InventoryWearLevel>( world_carry_item_entt, inventory_slot_level_cmp->m_level ); }
    reg.destroy( inventory_slot_cmp_entt );
    return world_carry_item_entt;
  }
}

//! @brief Remove the CarryItem from the world and add it to the player inventory
//! @param reg the ECS registry
//! @param carryitem_entt the CarryItem entt from the world
//! @return entt::entity
entt::entity pickupCarryItem( entt::registry &reg, entt::entity carryitem_entt )
{
  auto carryitem_cmp = reg.try_get<Cmp::CarryItem>( carryitem_entt );
  auto carryitem_slot_level_cmp = reg.try_get<Cmp::InventoryWearLevel>( carryitem_entt );

  if ( not carryitem_cmp ) return entt::null;

  auto inventory_entity = reg.create();
  reg.emplace<Cmp::PlayerInventorySlot>( inventory_entity, carryitem_cmp->type );
  reg.emplace<Cmp::SpriteAnimation>( inventory_entity, 0, 0, false, carryitem_cmp->type, 0 );
  if ( carryitem_slot_level_cmp ) { reg.emplace_or_replace<Cmp::InventoryWearLevel>( inventory_entity, carryitem_slot_level_cmp->m_level ); }

  reg.destroy( carryitem_entt );

  return inventory_entity;
}

//! @brief Destroy all player inventory slots matching a type. See "CARRYITEM.xxxx" in res/json/sprite_metadata.json
//! @param reg the ECS registry
//! @param type the type to destroy
void destroyInventory( entt::registry &reg, const Sprites::SpriteMetaType type )
{
  auto inventory_view = reg.view<Cmp::PlayerInventorySlot>();
  for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
  {
    if ( inventory_cmp.type == type ) reg.destroy( inventory_entt );
  }
}

} // namespace ProceduralMaze::Factory
