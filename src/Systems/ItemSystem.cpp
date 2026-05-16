
#include <Audio/SoundBank.hpp>
#include <Components/AnimData.hpp>
#include <Components/Inventory/ScryingBall.hpp>
#include <Components/Position.hpp>
#include <Events/CreateItemEvent.hpp>
#include <Inventory/Explosive.hpp>
#include <Inventory/InventoryWearLevel.hpp>
#include <Npc/NpcNoPathFinding.hpp>
#include <ReservedPosition.hpp>
#include <Systems/ItemSystem.hpp>
#include <Systems/Stores/ItemStore.hpp>
#include <UUID.hpp>
#include <ZOrderValue.hpp>

namespace ProceduralMaze::Sys
{

ItemSystem::ItemSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "ItemSystem initialized" );
  std::ignore = get_systems_event_queue().sink<Events::CreateItemEvent>().connect<&ItemSystem::on_create_item_event>( this );
}

void ItemSystem::on_create_item_event( ProceduralMaze::Events::CreateItemEvent ev )
{
  create_world_item( ev.m_pos, ev.m_item, ev.m_sfx, ev.m_zorder );
}

void ItemSystem::create_world_item( Cmp::Position pos, const std::string &item, std::string sfx, float zorder )
{
  if ( item == "item.seeingstone" )
  {
    create_seeing_stone( pos, item, zorder );
    return;
  }
  if ( item == "item.bomb" )
  {
    create_explosive( pos, item, zorder );
    return;
  }

  auto world_item_entt = reg().create();
  reg().emplace_or_replace<Cmp::Position>( world_item_entt, pos.position, pos.size );
  reg().emplace_or_replace<Cmp::ReservedPosition>( world_item_entt );
  // clang-format off
  reg().emplace_or_replace<Cmp::AnimData>( world_item_entt, Cmp::AnimData::Config{ 
        .sprite_type =  Sys::ItemStore::instance().get_item( item ).sprite_type, 
        .enabled = true
  });
  // clang-format on
  reg().emplace_or_replace<Cmp::ZOrderValue>( world_item_entt, pos.position.y - 1.f + zorder );
  reg().emplace_or_replace<Cmp::NpcNoPathFinding>( world_item_entt );
  // Use a UUID to identify the InventoryItem/PlayerInventorySlot when the entity is destroyed.
  reg().emplace_or_replace<Cmp::UUID>( world_item_entt, Cmp::UUID::generate() );
  if ( item == "item.axe" || item == "item.pickaxe" || item == "item.shovel" )
  {
    reg().emplace_or_replace<Cmp::InventoryWearLevel>( world_item_entt, 100.f );
  }
  reg().emplace_or_replace<Cmp::WorldItem>( world_item_entt, Sys::ItemStore::instance().get_item( item ) );

  SPDLOG_INFO( "Placed {} at {},{}", item, pos.position.x, pos.position.y );
  if ( world_item_entt != entt::null and not sfx.empty() ) { m_sound_bank.get_effect( sfx ).play(); }
}

void ItemSystem::create_seeing_stone( Cmp::Position pos, const std::string &item, float zorder )
{
  // Check if we can create a component with a unique target BEFORE creating the entity
  std::vector<Cmp::SeeingStone::Target> exclude_list;
  for ( auto [scryingball_entt, scryingball_cmp] : reg().view<Cmp::SeeingStone>().each() )
  {
    exclude_list.push_back( scryingball_cmp.target );
  }
  auto pick = Cmp::SeeingStone::random_pick( exclude_list );
  if ( pick == Cmp::SeeingStone::Target::NONE )
  {
    SPDLOG_WARN( "Cannot create scrying ball - all targets already assigned" );
    return;
  }

  // Now create the entity with the valid target
  auto world_carry_item_entt = reg().create();
  reg().emplace_or_replace<Cmp::Position>( world_carry_item_entt, pos.position, pos.size );
  reg().emplace_or_replace<Cmp::ReservedPosition>( world_carry_item_entt );
  // clang-format off
  reg().emplace_or_replace<Cmp::AnimData>( world_carry_item_entt, Cmp::AnimData::Config{  
        .sprite_type =  Sys::ItemStore::instance().get_item( item ).sprite_type
  });
  // clang-format on
  reg().emplace_or_replace<Cmp::ZOrderValue>( world_carry_item_entt, pos.position.y - 1.f + zorder );
  reg().emplace_or_replace<Cmp::WorldItem>( world_carry_item_entt, Sys::ItemStore::instance().get_item( item ) );
  reg().emplace_or_replace<Cmp::NpcNoPathFinding>( world_carry_item_entt );
  reg().emplace_or_replace<Cmp::SeeingStone>( world_carry_item_entt, false, pick );

  SPDLOG_INFO( "Placed {} at {},{}", item, pos.position.x, pos.position.y );
}

void ItemSystem::create_explosive( Cmp::Position pos, const std::string &item, float zorder )
{
  // Now create the entity with the valid target
  auto world_carry_item_entt = reg().create();
  reg().emplace_or_replace<Cmp::Position>( world_carry_item_entt, pos.position, pos.size );
  // clang-format off
  reg().emplace_or_replace<Cmp::AnimData>( world_carry_item_entt, Cmp::AnimData::Config{  
        .sprite_type =  Sys::ItemStore::instance().get_item( item ).sprite_type
  });
  //clang-format on
  reg().emplace_or_replace<Cmp::ZOrderValue>( world_carry_item_entt, pos.position.y - 1.f + zorder );
  reg().emplace_or_replace<Cmp::WorldItem>( world_carry_item_entt, Sys::ItemStore::instance().get_item( item ) );
  reg().emplace_or_replace<Cmp::NpcNoPathFinding>( world_carry_item_entt );
  reg().emplace_or_replace<Cmp::Explosive>( world_carry_item_entt, false );

  SPDLOG_INFO( "Placed {} at {},{}", item, pos.position.x, pos.position.y );
}

} // namespace ProceduralMaze::Sys