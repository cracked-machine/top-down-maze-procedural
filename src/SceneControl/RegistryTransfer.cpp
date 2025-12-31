#include <Components/CryptLever.hpp>
#include <Components/CryptObjectiveMultiBlock.hpp>
#include <Components/CryptPassageBlock.hpp>
#include <SceneControl/RegistryTransfer.hpp>

#include <spdlog/spdlog.h>

#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Armable.hpp>
#include <Components/CryptRoomClosed.hpp>
#include <Components/CryptRoomEnd.hpp>
#include <Components/CryptRoomOpen.hpp>
#include <Components/CryptRoomStart.hpp>
#include <Components/Direction.hpp>
#include <Components/FootStepAlpha.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/Neighbours.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PCDetectionBounds.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCadaverCount.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/WeaponLevel.hpp>
#include <Components/ZOrderValue.hpp>

namespace ProceduralMaze::Scene
{

RegistryTransfer::RegCopy RegistryTransfer::copy_reg( IScene &scene, Scene::RegCopyMode copy_mode )
{
  if ( copy_mode == RegCopyMode::NONE ) { return nullptr; }

  auto registry_copy = std::make_unique<entt::registry>();
  auto &source_registry = scene.registry();

  ensure_player_component_storages( *registry_copy );

  int skipped_cmp = 0;
  std::vector<std::string> copied_cmp;
  for ( auto entity : source_registry.storage<entt::entity>() )
  {
    // this is a list of components that we do NOT want to copy over
    if ( source_registry.any_of<Cmp::ReservedPosition, Cmp::Obstacle, Cmp::Armable, Cmp::Neighbours, Cmp::NoPathFinding, Cmp::FootStepTimer,
                                Cmp::FootStepAlpha, Cmp::CryptRoomOpen, Cmp::CryptRoomClosed, Cmp::CryptRoomStart, Cmp::CryptRoomEnd,
                                Cmp::CryptPassageBlock, Cmp::CryptLever, Cmp::CryptObjectiveMultiBlock>( entity ) )
    {
      skipped_cmp++;
      continue; // Skip player entity
    }
    auto new_entity = registry_copy->create();

    // Copy all components from this entity
    for ( auto &&curr : source_registry.storage() )
    {
      if ( auto &source_storage = curr.second; source_storage.contains( entity ) )
      {
        auto type_hash = curr.first;
        // Ensure storage exists in target registry
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
  SPDLOG_INFO( "Registry copy completed: {} copied, {} skipped", copied_cmp.size(), skipped_cmp );
  pretty_print( copied_cmp );

  return registry_copy;
}

void RegistryTransfer::xfer_player_entt( entt::registry &from_registry, entt::registry &to_registry )
{
  auto player_view = from_registry.view<Cmp::PlayableCharacter>();
  if ( player_view.empty() )
  {
    SPDLOG_WARN( "No player entity found to transfer" );
    return;
  }

  auto source_entity = player_view.front();

  // Check if player entity already exists in target registry
  auto target_player_view = to_registry.view<Cmp::PlayableCharacter>();
  entt::entity target_entity;

  if ( target_player_view.empty() )
  {
    // No player exists, create new one
    target_entity = to_registry.create();
    SPDLOG_INFO( "Created new player entity (#{}) in target registry", static_cast<uint32_t>( target_entity ) );
  }
  else
  {
    // Player exists, use existing entity
    target_entity = target_player_view.front();
    SPDLOG_INFO( "Using existing player entity (#{}) in target registry", static_cast<uint32_t>( target_entity ) );
  }

  // Ensure all known player component storages exist in target registry
  ensure_player_component_storages( to_registry );

  // Create a copy of an entity component by component (from entt wiki)
  std::vector<std::string> transferred_cmps;
  std::vector<std::string> removed_cmps;
  for ( auto &&curr : from_registry.storage() )
  {
    if ( auto &source_storage = curr.second; source_storage.contains( source_entity ) )
    {
      SPDLOG_DEBUG( "Transferring component: {}", source_storage.type().name() );

      auto type_hash = curr.first;

      if ( auto *target_storage = to_registry.storage( type_hash ) )
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
      else { SPDLOG_WARN( "No storage found in target reg for cmp: {}", source_storage.type().name() ); }
    }
  }
  SPDLOG_INFO( "Component transfer completed: {} removed", removed_cmps.size() );
  pretty_print( removed_cmps );
  SPDLOG_INFO( "Component transfer completed: {} transferred", transferred_cmps.size() );
  pretty_print( transferred_cmps );
}

void RegistryTransfer::ensure_player_component_storages( entt::registry &registry )
{
  // Force storage creation by accessing storage for each known component type
  registry.storage<Cmp::AbsoluteAlpha>();
  registry.storage<Cmp::AbsoluteRotation>();
  registry.storage<Cmp::Direction>();
  registry.storage<Cmp::PCDetectionBounds>();
  registry.storage<Cmp::Position>();
  registry.storage<Cmp::PlayerDistance>();
  registry.storage<Cmp::PlayableCharacter>();
  registry.storage<Cmp::PlayerHealth>();
  registry.storage<Cmp::PlayerKeysCount>();
  registry.storage<Cmp::PlayerCandlesCount>();
  registry.storage<Cmp::PlayerMortality>();
  registry.storage<Cmp::PlayerRelicCount>();
  registry.storage<Cmp::PlayerCadaverCount>();
  registry.storage<Cmp::SpriteAnimation>();
  registry.storage<Cmp::WeaponLevel>();
  registry.storage<Cmp::ZOrderValue>();
  registry.storage<Cmp::System>();
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