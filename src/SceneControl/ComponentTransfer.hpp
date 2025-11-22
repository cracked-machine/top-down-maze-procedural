#ifndef SRC_SCENECONTROL_COMPONENTTRANSFER_HPP_
#define SRC_SCENECONTROL_COMPONENTTRANSFER_HPP_

#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <SceneControl/SceneStack.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Scene
{

class ComponentTransfer
{
public:
  enum class TransferMode
  {
    NONE,
    PLAYER_INVENTORY
  };

  using RegistrySnapshot = std::unique_ptr<entt::registry>;

  // Helper function to capture the current scene's registry
  RegistrySnapshot capture( IScene &scene, TransferMode transfer_mode = TransferMode::NONE )
  {
    if ( transfer_mode == TransferMode::NONE ) { return nullptr; }
    auto registry_copy = std::make_unique<entt::registry>();
    registry_copy->swap( scene.get_registry() );
    return registry_copy;
  }

  // Helper function to transfer player inventory components between registries
  void transfer_player_inventory( entt::registry &from_registry, entt::registry &to_registry )
  {
    transfer_components<Cmp::PlayerKeysCount, Cmp::PlayerCandlesCount, Cmp::PlayerRelicCount>( from_registry, to_registry );
  }

private:
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