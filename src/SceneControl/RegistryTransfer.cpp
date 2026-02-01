#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <Components/Crypt/CryptLever.hpp>
#include <Components/Crypt/CryptObjectiveMultiBlock.hpp>
#include <Components/Crypt/CryptPassageBlock.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Player/PlayerBlastRadius.hpp>
#include <Components/Player/PlayerLastGraveyardPosition.hpp>
#include <Components/Player/PlayerWealth.hpp>
#include <Components/Ruin/RuinObjectiveType.hpp>
#include <SceneControl/RegistryTransfer.hpp>

#include <spdlog/spdlog.h>

#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Armable.hpp>
#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/Direction.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Inventory/InventoryWearLevel.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Player/PlayerCadaverCount.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerDetectionBounds.hpp>
#include <Components/Player/PlayerDistance.hpp>
#include <Components/Player/PlayerHealth.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/ZOrderValue.hpp>

namespace ProceduralMaze::Scene
{

//! @brief Creates a deep copy of selected entities from a scene's ECS registry into a new standalone registry. The function supports different copy
//! modes to control what gets transferred during scene transitions — either just the player entity's state, all non-blacklisted entities, or nothing
//! at all.
//! @details
//! 1. Early exit check — Returns nullptr if copy_mode is NONE
//! 2. Initialize target registry — Creates a new entt::registry and pre-registers known component storages via init_missing_cmp_storages()
//! 3. Copy player entity (for PLAYER_ONLY or ALL modes):
//!   Find the entity with PlayerCharacter component
//!   Create a new entity in the target registry
//!   Iterate all component storages and copy each component the player has
//!   Skip components without registered storage (e.g., LerpPosition)
//! 4. Copy other entities (for ALL mode only):
//!   Iterate all entities in source registry
//!   Skip the player (already copied)
//!   Skip entities with blacklisted components (map/crypt-specific data)
//!   For each valid entity, create a new entity and copy all its components
//! @param scene
//! @param copy_mode
//! @return RegistryTransfer::RegCopy
RegistryTransfer::RegCopy RegistryTransfer::copy_reg( IScene &scene, Scene::RegCopyMode copy_mode )
{
  if ( copy_mode == RegCopyMode::NONE ) { return nullptr; }

  auto registry_copy = std::make_unique<entt::registry>();
  auto &source_registry = scene.registry();

  init_missing_cmp_storages( *registry_copy );

  [[maybe_unused]] int skipped_cmp = 0;
  std::vector<std::string> copied_cmp;

  // Copy player entity (for PLAYER_ONLY and ALL modes)
  if ( copy_mode == RegCopyMode::PLAYER_ONLY || copy_mode == RegCopyMode::ALL )
  {
    auto player_view = source_registry.view<Cmp::PlayerCharacter>();
    if ( !player_view.empty() )
    {
      auto player_entity = player_view.front();
      auto new_entity = registry_copy->create();

      // Copy all components from player entity
      for ( auto &&curr : source_registry.storage() )
      {
        if ( auto &source_storage = curr.second; source_storage.contains( player_entity ) )
        {
          auto type_hash = curr.first;
          if ( auto *target_storage = registry_copy->storage( type_hash ) )
          {
            target_storage->push( new_entity, source_storage.value( player_entity ) );
            SPDLOG_DEBUG( "Copied player component: {}", source_storage.type().name() );
            copied_cmp.emplace_back( source_storage.type().name() );
          }
          else { SPDLOG_WARN( "No storage found in target registry for component: {}", source_storage.type().name() ); }
        }
      }
      SPDLOG_DEBUG( "Player entity copied with {} components", copied_cmp.size() );
    }
    else { SPDLOG_WARN( "No player entity found in source registry to copy" ); }
  }

  // Copy other entities (for ALL mode only)
  if ( copy_mode == RegCopyMode::ALL )
  {
    for ( auto entity : source_registry.storage<entt::entity>() )
    {
      // Skip player entity (already copied above)
      if ( source_registry.any_of<Cmp::PlayerCharacter>( entity ) ) { continue; }

      // Skip blacklisted components
      if ( source_registry.any_of<Cmp::ReservedPosition, Cmp::Obstacle, Cmp::Armable, Cmp::Neighbours, Cmp::NpcNoPathFinding, Cmp::FootStepTimer,
                                  Cmp::FootStepAlpha, Cmp::CryptRoomOpen, Cmp::CryptRoomClosed, Cmp::CryptRoomStart, Cmp::CryptRoomEnd,
                                  Cmp::CryptPassageBlock, Cmp::CryptLever, Cmp::CryptObjectiveMultiBlock>( entity ) )
      {
        skipped_cmp++;
        continue;
      }

      auto new_entity = registry_copy->create();

      for ( auto &&curr : source_registry.storage() )
      {
        if ( auto &source_storage = curr.second; source_storage.contains( entity ) )
        {
          auto type_hash = curr.first;
          if ( auto *target_storage = registry_copy->storage( type_hash ) )
          {
            target_storage->push( new_entity, source_storage.value( entity ) );
            SPDLOG_DEBUG( "Copied component: {}", source_storage.type().name() );
            copied_cmp.emplace_back( source_storage.type().name() );
          }
          else { SPDLOG_WARN( "No storage found in target registry for component: {}", source_storage.type().name() ); }
        }
      }
    }
  }

  SPDLOG_DEBUG( "Registry copy completed: {} copied, {} skipped", copied_cmp.size(), skipped_cmp );
  pretty_print( copied_cmp );

  return registry_copy;
}

void RegistryTransfer::xfer_player_entt( entt::registry &source_registry, entt::registry &target_registry )
{
  auto player_view = source_registry.view<Cmp::PlayerCharacter>();
  if ( player_view.empty() )
  {
    SPDLOG_WARN( "No player entity found to transfer" );
    return;
  }

  auto source_entity = player_view.front();

  // Check if player entity already exists in target registry
  auto target_player_view = target_registry.view<Cmp::PlayerCharacter>();
  entt::entity target_entity;

  if ( target_player_view.empty() )
  {
    // No player exists, create new one
    target_entity = target_registry.create();
    SPDLOG_DEBUG( "Created new player entity (#{}) in target registry", static_cast<uint32_t>( target_entity ) );
  }
  else
  {
    // Player exists, use existing entity
    target_entity = target_player_view.front();
    SPDLOG_DEBUG( "Using existing player entity (#{}) in target registry", static_cast<uint32_t>( target_entity ) );
  }

  // Ensure all known player component storages exist in target registry
  init_missing_cmp_storages( target_registry );

  // Create a copy of an entity component by component (from entt wiki)
  std::vector<std::string> transferred_cmps;
  std::vector<std::string> removed_cmps;
  for ( auto &&curr : source_registry.storage() )
  {
    if ( auto &source_storage = curr.second; source_storage.contains( source_entity ) )
    {
      SPDLOG_DEBUG( "Transferring component: {}", source_storage.type().name() );

      auto type_hash = curr.first;

      if ( auto *target_storage = target_registry.storage( type_hash ) )
      {
        if ( target_storage->contains( target_entity ) )
        {
          target_storage->erase( target_entity );
          SPDLOG_DEBUG( "Removed existing component: {}", source_storage.type().name() );
          removed_cmps.emplace_back( source_storage.type().name() );
        }
        target_storage->push( target_entity, source_storage.value( source_entity ) );
        transferred_cmps.emplace_back( source_storage.type().name() );

        SPDLOG_DEBUG( "Successfully transferred component: {}", source_storage.type().name() );
      }
      else { SPDLOG_DEBUG( "No storage found in target reg for cmp: {}", source_storage.type().name() ); }
    }
  }
  SPDLOG_DEBUG( "Component transfer completed: {} removed", removed_cmps.size() );
  pretty_print( removed_cmps );
  SPDLOG_DEBUG( "Component transfer completed: {} transferred", transferred_cmps.size() );
  pretty_print( transferred_cmps );
}

void RegistryTransfer::init_missing_cmp_storages( entt::registry &registry )
{
  // Force storage creation by accessing storage for each known component type
  registry.storage<Cmp::AbsoluteAlpha>();
  registry.storage<Cmp::AbsoluteRotation>();
  registry.storage<Cmp::Direction>();
  registry.storage<Cmp::PCDetectionBounds>();
  registry.storage<Cmp::Position>();
  registry.storage<Cmp::PlayerDistance>();
  registry.storage<Cmp::PlayerCharacter>();
  registry.storage<Cmp::PlayerHealth>();
  registry.storage<Cmp::PlayerWealth>();
  registry.storage<Cmp::PlayerBlastRadius>();
  registry.storage<Cmp::PlayerMortality>();
  registry.storage<Cmp::PlayerCadaverCount>();
  registry.storage<Cmp::SpriteAnimation>();
  registry.storage<Cmp::PlayerInventorySlot>();
  registry.storage<Cmp::CarryItem>();
  registry.storage<Cmp::InventoryWearLevel>();
  registry.storage<Cmp::ZOrderValue>();
  registry.storage<Cmp::System>();
  registry.storage<Cmp::PlayerLastGraveyardPosition>();
  registry.storage<Cmp::PlayerRuinLocation>();
  registry.storage<Cmp::RuinObjectiveType>();
  // Add other player-related components as needed
}

void RegistryTransfer::pretty_print( const std::vector<std::string> &components )
{
  if ( components.empty() ) return;

  std::stringstream ss;
  ss << "[ ";
  for ( const auto &comp_name : components )
  {
    // get class from namespace string after last "::"
    auto last_colon = comp_name.find_last_of( "::" );
    if ( last_colon != std::string::npos ) { ss << comp_name.substr( last_colon + 1 ) << " "; }
    else { ss << comp_name << " "; }
  }
  ss << "]";
  SPDLOG_DEBUG( "{}", ss.str() );
}

} // namespace ProceduralMaze::Scene