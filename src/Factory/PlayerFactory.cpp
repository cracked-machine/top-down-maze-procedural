#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteOffset.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/DeathPosition.hpp>
#include <Components/Direction.hpp>
#include <Components/Inventory/Explosive.hpp>
#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/Inventory/ScryingBall.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/PlayerBlastRadius.hpp>
#include <Components/Player/PlayerCadaverCount.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/Player/PlayerWealth.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Stats/PlayerStats.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Player/PlayerCurse.hpp>
#include <Player/PlayerLevelDepth.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Stats/PlayerStats.hpp>
#include <Stats/SacrificeAction.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Stores/ItemStore.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Factory
{

void create_player( entt::registry &reg )
{
  SPDLOG_INFO( "Creating player entity" );
  auto entity = reg.create();

  // start position must be pixel coordinates within the screen resolution (kDisplaySize),
  // but also grid aligned (kMapGridSize) to avoid collision detection errors.
  // So we must recalc start position to the nearest grid position here
  auto start_pos = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( reg );
  start_pos = Utils::snap_to_grid( start_pos );
  reg.emplace_or_replace<Cmp::Position>( entity, start_pos, Constants::kGridSizePxF );
  auto &blast_radius = Sys::PersistSystem::get<Cmp::Persist::BlastRadius>( reg );
  reg.emplace_or_replace<Cmp::PlayerCharacter>( entity );
  reg.emplace_or_replace<Cmp::ReservedPosition>( entity );
  reg.emplace_or_replace<Cmp::PlayerBlastRadius>( entity, blast_radius.get_value() );
  reg.emplace_or_replace<Cmp::PlayerStats>( entity, Cmp::Stats::Health{ 100 }, Cmp::Stats::Fear{ 0 }, Cmp::Stats::Despair{ 0 },
                                            Cmp::Stats::Infamy{ 0 } );

  reg.emplace_or_replace<Cmp::Direction>( entity, sf::Vector2f{ 0, 0 } );

  reg.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "sprite.player.walk.south" );
  reg.emplace_or_replace<Cmp::PlayerCadaverCount>( entity, 0 );
  reg.emplace_or_replace<Cmp::PlayerWealth>( entity, 0 );
  reg.emplace_or_replace<Cmp::PlayerMortality>( entity, Cmp::PlayerMortality::State::ALIVE );
  reg.emplace_or_replace<Cmp::PlayerCurse>( entity, false );
  reg.emplace_or_replace<Cmp::PlayerLevelDepth>( entity, 1 );

  reg.emplace_or_replace<Cmp::ZOrderValue>( entity, start_pos.y ); // z-order based on y-position
  reg.emplace_or_replace<Cmp::AbsoluteAlpha>( entity, 255 );       // fully opaque
  reg.emplace_or_replace<Cmp::AbsoluteRotation>( entity, 0 );
  add_inventory( reg, "item.pickaxe" );
}

void add_spawn_area( entt::registry &registry, entt::entity entity, Sprites::SpriteFactory &sfactory, float zorder )
{
  // We need to reserve these positions for the player start area
  registry.emplace_or_replace<Cmp::ReservedPosition>( entity );
  registry.emplace_or_replace<Cmp::SpawnArea>( entity, false );
  registry.emplace_or_replace<Cmp::NpcNoPathFinding>( entity );
  auto [_, idx] = sfactory.get_random_type_and_texture_index( { "sprite.graveyard.playerspawn" } );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "sprite.graveyard.playerspawn", idx );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entity, zorder );
}

void create_player_death_anim( entt::registry &registry, Cmp::Position player_pos_cmp, const Sprites::MultiSprite &sprite )
{
  auto player_blood_splat_entity = registry.create();
  sf::Vector2f offset;
  if ( ( sprite.getSpriteSizePixels().x == Constants::kGridSizePxF.x ) and ( sprite.getSpriteSizePixels().y == Constants::kGridSizePxF.y ) )
  {
    offset = sf::Vector2f{ 0, 0 };
  }
  else { offset = sprite.getSpriteSizePixels() / 2.f; }
  registry.emplace_or_replace<Cmp::Position>( player_blood_splat_entity, player_pos_cmp.position - offset, player_pos_cmp.size );
  registry.emplace_or_replace<Cmp::DeathPosition>( player_blood_splat_entity, player_pos_cmp.position - offset, player_pos_cmp.size );
  registry.emplace_or_replace<Cmp::SpriteAnimation>( player_blood_splat_entity, 0, 0, true, sprite.get_sprite_type(), 0, 0.1,
                                                     Cmp::AnimType::ONESHOTHOLD );
  registry.emplace_or_replace<Cmp::ZOrderValue>( player_blood_splat_entity, player_pos_cmp.position.y * 3 ); // always infront
}

entt::entity create_seeing_stone( entt::registry &reg, Cmp::Position pos, const std::string &item, float zorder )
{
  // Check if we can create a component with a unique target BEFORE creating the entity
  std::vector<Cmp::SeeingStone::Target> exclude_list;
  for ( auto [scryingball_entt, scryingball_cmp] : reg.view<Cmp::SeeingStone>().each() )
  {
    exclude_list.push_back( scryingball_cmp.target );
  }
  auto pick = Cmp::SeeingStone::random_pick( exclude_list );
  if ( pick == Cmp::SeeingStone::Target::NONE )
  {
    SPDLOG_WARN( "Cannot create scrying ball - all targets already assigned" );
    return entt::null;
  }

  // Now create the entity with the valid target
  auto world_carry_item_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( world_carry_item_entt, pos.position, pos.size );
  reg.emplace_or_replace<Cmp::ReservedPosition>( world_carry_item_entt );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( world_carry_item_entt, 0, 0, true, Sys::ItemStore::instance().get_item( item ).sprite_type, 0 );
  reg.emplace_or_replace<Cmp::ZOrderValue>( world_carry_item_entt, pos.position.y - 1.f + zorder );
  reg.emplace_or_replace<Cmp::InventoryItem>( world_carry_item_entt, Sys::ItemStore::instance().get_item( item ) );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( world_carry_item_entt );
  reg.emplace_or_replace<Cmp::SeeingStone>( world_carry_item_entt, false, pick );

  SPDLOG_INFO( "Placed {} at {},{}", item, pos.position.x, pos.position.y );
  return world_carry_item_entt;
}

entt::entity create_explosive( entt::registry &reg, Cmp::Position pos, const std::string &item, float zorder )
{
  // Now create the entity with the valid target
  auto world_carry_item_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( world_carry_item_entt, pos.position, pos.size );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( world_carry_item_entt, 0, 0, true, Sys::ItemStore::instance().get_item( item ).sprite_type, 0 );
  reg.emplace_or_replace<Cmp::ZOrderValue>( world_carry_item_entt, pos.position.y - 1.f + zorder );
  reg.emplace_or_replace<Cmp::InventoryItem>( world_carry_item_entt, Sys::ItemStore::instance().get_item( item ) );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( world_carry_item_entt );
  reg.emplace_or_replace<Cmp::Explosive>( world_carry_item_entt, false );

  SPDLOG_INFO( "Placed {} at {},{}", item, pos.position.x, pos.position.y );
  return world_carry_item_entt;
}

entt::entity create_world_item( entt::registry &reg, Cmp::Position pos, const std::string &item, float zorder )
{
  if ( item == "item.seeingstone" ) { return create_seeing_stone( reg, pos, item, zorder ); }
  if ( item == "item.bomb" ) { return create_explosive( reg, pos, item, zorder ); }

  auto world_item_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( world_item_entt, pos.position, pos.size );
  reg.emplace_or_replace<Cmp::ReservedPosition>( world_item_entt );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( world_item_entt, 0, 0, true, Sys::ItemStore::instance().get_item( item ).sprite_type, 0 );
  reg.emplace_or_replace<Cmp::ZOrderValue>( world_item_entt, pos.position.y - 1.f + zorder );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( world_item_entt );
  if ( item == "item.axe" || item == "item.pickaxe" || item == "item.shovel" )
  {
    reg.emplace_or_replace<Cmp::InventoryWearLevel>( world_item_entt, 100.f );
  }
  reg.emplace_or_replace<Cmp::InventoryItem>( world_item_entt, Sys::ItemStore::instance().get_item( item ) );

  SPDLOG_INFO( "Placed {} at {},{}", item, pos.position.x, pos.position.y );
  return world_item_entt;
}

entt::entity pickup_world_item( entt::registry &reg, entt::entity world_item_entt )
{
  // does this entity own a world item that can be carried?
  auto *world_item_cmp = reg.try_get<Cmp::InventoryItem>( world_item_entt );
  if ( not world_item_cmp ) return entt::null;

  // create the basic inventory slot entt
  auto inventory_entity = reg.create();
  reg.emplace_or_replace<Cmp::PlayerInventorySlot>( inventory_entity, *world_item_cmp );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( inventory_entity, 0, 0, false, world_item_cmp->sprite_type, 0 );

  // transfer any component properties from the world item that we want to retain before it is destroyed
  auto *wear_level_cmp = reg.try_get<Cmp::InventoryWearLevel>( world_item_entt );
  if ( wear_level_cmp ) { reg.emplace_or_replace<Cmp::InventoryWearLevel>( inventory_entity, wear_level_cmp->m_level ); }

  auto *scryingball_cmp = reg.try_get<Cmp::SeeingStone>( world_item_entt );
  if ( scryingball_cmp ) { reg.emplace_or_replace<Cmp::SeeingStone>( inventory_entity, false, scryingball_cmp->target ); }

  auto *explosive_cmp = reg.try_get<Cmp::Explosive>( world_item_entt );
  if ( explosive_cmp ) { reg.emplace_or_replace<Cmp::Explosive>( inventory_entity, false ); }

  // now destroy the world item entt
  SPDLOG_INFO( "Picked up world entt {}", static_cast<uint32_t>( world_item_entt ) );
  reg.destroy( world_item_entt );

  return inventory_entity;
}

void add_inventory( entt::registry &reg, const std::string &item )
{
  auto inventory_entity = reg.create();
  reg.emplace_or_replace<Cmp::PlayerInventorySlot>( inventory_entity, Sys::ItemStore::instance().get_item( item ) );
  if ( item.contains( "axe" ) or item.contains( "shovel" ) ) { reg.emplace_or_replace<Cmp::InventoryWearLevel>( inventory_entity, 100.f ); }
  if ( item.contains( "scryingball" ) )
  {
    Cmp::SeeingStone sb;
    sb.target = ProceduralMaze::Cmp::SeeingStone::random_pick( {} );
    reg.emplace_or_replace<Cmp::SeeingStone>( inventory_entity, sb );
  }
  if ( item.contains( "explosive" ) ) { reg.emplace_or_replace<Cmp::Explosive>( inventory_entity, false ); }
  reg.emplace_or_replace<Cmp::SpriteAnimation>( inventory_entity, 0, 0, true, Sys::ItemStore::instance().get_item( item ).sprite_type, 0 );
}

void destroy_inventory( entt::registry &reg, const Sprites::SpriteMetaType &type )
{
  auto inventory_view = reg.view<Cmp::PlayerInventorySlot>();
  for ( auto [inventory_entt, inventory_cmp] : inventory_view.each() )
  {
    if ( inventory_cmp.m_item.sprite_type == type ) reg.destroy( inventory_entt );
  }
}

Cmp::Position add_player_last_graveyard_pos( entt::registry &reg, Cmp::Position &last_known_pos, [[maybe_unused]] sf::Vector2f offset )
{
  SPDLOG_INFO( "Player will re-enter grave yard at {},{}", last_known_pos.position.x, last_known_pos.position.y );
  auto player_entt = Utils::Player::get_entity( reg );
  reg.emplace_or_replace<Cmp::PlayerLastGraveyardPosition>( player_entt, last_known_pos.position, last_known_pos.size );
  return last_known_pos;
}

void remove_player_last_graveyard_pos( entt::registry &reg )
{
  auto player_entt = Utils::Player::get_entity( reg );
  reg.remove<Cmp::PlayerLastGraveyardPosition>( player_entt );
}

} // namespace ProceduralMaze::Factory
