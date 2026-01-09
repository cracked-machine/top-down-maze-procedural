#ifndef SRC_SCENECONTROL_COMPONENTTRANSFER_HPP_
#define SRC_SCENECONTROL_COMPONENTTRANSFER_HPP_

#include <SceneControl/SceneStack.hpp>
#include <entt/entity/fwd.hpp>
#include <memory>

namespace ProceduralMaze::Scene
{

//! @brief Enumeration for registry copy modes
enum class RegCopyMode {
  //! @brief Copy no components
  NONE,
  //! @brief Copy only player components
  PLAYER_ONLY
};

class RegistryTransfer
{
public:
  //! @brief Type alias for a unique pointer to an entt::registry
  using RegCopy = std::unique_ptr<entt::registry>;

  //! @brief Create a copy of the registry from the given scene
  //! @param scene The scene to copy the registry from
  //! @param copy_mode The mode to use for copying
  //! @return RegCopy A unique pointer to the copied registry
  RegCopy copy_reg( IScene &scene, RegCopyMode copy_mode = RegCopyMode::NONE );

  //! @brief Transfer player entity components from one registry to another
  //! @param from_registry Source registry
  //! @param to_registry Target registry
  void xfer_player_entt( entt::registry &from_registry, entt::registry &to_registry );
  void xfer_inventory_entt( entt::registry &from_registry, entt::registry &to_registry );

private:
  //! @brief Ensure all known player component storages exist in the given registry
  //! @param registry
  void ensure_player_component_storages( entt::registry &registry );

  //! @brief Print python style list of component type names
  //! @param components Vector of component type names
  void pretty_print( const std::vector<std::string> &components );
};

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_COMPONENTTRANSFER_HPP_