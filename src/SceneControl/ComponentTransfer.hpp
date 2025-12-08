#ifndef SRC_SCENECONTROL_COMPONENTTRANSFER_HPP_
#define SRC_SCENECONTROL_COMPONENTTRANSFER_HPP_

#include <Components/AbsoluteAlpha.hpp>
#include <Components/AbsoluteRotation.hpp>
#include <Components/Armable.hpp>
#include <Components/Direction.hpp>
#include <Components/Neighbours.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/PCDetectionBounds.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/PlayerHealth.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/WeaponLevel.hpp>
#include <Components/ZOrderValue.hpp>
#include <SceneControl/SceneStack.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Scene
{
enum class RegCopyMode
{
  NONE,
  PLAYER
};
class RegistryTransfer
{
public:
  using RegCopy = std::unique_ptr<entt::registry>;

  // Helper function to capture the current scene's registry
  RegCopy copy_reg( IScene &scene, RegCopyMode copy_mode = RegCopyMode::NONE )
  {
    if ( copy_mode == RegCopyMode::NONE ) { return nullptr; }

    auto registry_copy = std::make_unique<entt::registry>();
    auto &source_registry = scene.registry();

    // Ensure all known player component storages exist in target registry
    ensure_player_component_storages( *registry_copy );

    // Copy all entities and their components instead of swapping
    int copied_entities = 0;
    int skipped_entities = 0;
    for ( auto entity : source_registry.storage<entt::entity>() )
    {
      if ( source_registry.any_of<Cmp::ReservedPosition, Cmp::Obstacle, Cmp::Armable, Cmp::Neighbours, Cmp::NoPathFinding>( entity ) )
      {
        // SPDLOG_INFO( "Skipping copy of entity {}", static_cast<uint32_t>( entity ) );
        skipped_entities++;
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
            SPDLOG_INFO( "Copied component: {}", source_storage.type().name() );
            copied_entities++;
          }
          else { SPDLOG_WARN( "No storage found in target registry for component: {}", source_storage.type().name() ); }
        }
      }
    }
    SPDLOG_INFO( "Registry capture completed: {} entities copied, {} entities skipped", copied_entities, skipped_entities );

    return registry_copy;
  }

  // Transfer all components from player entity
  void xfer_player_entt( entt::registry &from_registry, entt::registry &to_registry )
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
      SPDLOG_INFO( "Created new player entity {} in target registry", static_cast<uint32_t>( target_entity ) );
    }
    else
    {
      // Player exists, use existing entity
      target_entity = target_player_view.front();
      SPDLOG_INFO( "Using existing player entity {} in target registry", static_cast<uint32_t>( target_entity ) );
    }

    // Ensure all known player component storages exist in target registry
    ensure_player_component_storages( to_registry );

    SPDLOG_INFO( "Transferring entity {} components", static_cast<uint32_t>( source_entity ) );

    // Create a copy of an entity component by component (from entt wiki)
    for ( auto &&curr : from_registry.storage() )
    {
      if ( auto &source_storage = curr.second; source_storage.contains( source_entity ) )
      {
        SPDLOG_INFO( "Transferring component: {}", source_storage.type().name() );

        auto type_hash = curr.first;

        if ( auto *target_storage = to_registry.storage( type_hash ) )
        {
          if ( target_storage->contains( target_entity ) )
          {
            target_storage->erase( target_entity );
            SPDLOG_INFO( "Removed existing component: {}", source_storage.type().name() );
          }
          target_storage->push( target_entity, source_storage.value( source_entity ) );
          SPDLOG_INFO( "Successfully transferred component: {}", source_storage.type().name() );
        }
        else { SPDLOG_WARN( "No storage found in target registry for component: {}", source_storage.type().name() ); }
      }
    }
  }

private:
  // Ensure storages exist for known player components
  void ensure_player_component_storages( entt::registry &registry )
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
    registry.storage<Cmp::SpriteAnimation>();
    registry.storage<Cmp::WeaponLevel>();
    registry.storage<Cmp::ZOrderValue>();
    // Add other player-related components as needed
  }

  // Generic component transfer function
  template <typename... Components>
  static void transfer_components( entt::registry &from_registry, entt::registry &to_registry )
  {
    auto transfer_component = [&]<typename Component>()
    {
      auto view = from_registry.view<Component>();
      if ( view.empty() )
      {
        SPDLOG_INFO( "No {} components to transfer", typeid( Component ).name() );
        return;
      }
      for ( auto entity : view )
      {
        const auto &component = view.template get<Component>( entity );
        auto new_entity = to_registry.create();
        to_registry.emplace<Component>( new_entity, component );
        SPDLOG_INFO( "Transferred component: {}", typeid( Component ).name() );
      }
    };

    ( transfer_component.template operator()<Components>(), ... );
  }
};

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_COMPONENTTRANSFER_HPP_