#ifndef _INC_PERSISTENT_SYSTEM_HPP
#define _INC_PERSISTENT_SYSTEM_HPP

#include <BaseSystem.hpp>
#include <Events/SaveSettingsEvent.hpp>

namespace ProceduralMaze::Sys {

class PersistentSystem : public BaseSystem
{
public:
  PersistentSystem( SharedEnttRegistry reg );

  /**
   * @brief Loads the persistent state data from storage.
   *
   * This function retrieves previously saved state information and restores
   * the system to its last known configuration. The state data typically
   * includes settings, progress, or other persistent information that should
   * survive between application sessions.
   *
   * @throws std::runtime_error if the state file cannot be read or is corrupted
   * @throws std::ios_base::failure if file I/O operations fail
   */
  void load_state();

  /**
   * @brief Saves the current state data to persistent storage.
   *
   * This function serializes the current configuration and state information
   * of the system and writes it to a file or database. This ensures that
   * important settings and progress are preserved for future sessions.
   *
   * @throws std::ios_base::failure if file I/O operations fail
   */
  void save_state();

  /// EVENTS
  void on_save_settings_event( [[maybe_unused]] const Events::SaveSettingsEvent &event )
  {
    SPDLOG_INFO( "Save Settings Event received" );
    save_state();
  }

private:
  /**
   * @brief Map of component loader functions indexed by component type name.
   *
   * This container stores function pointers that handle loading specific component types
   * from JSON data. Each key represents a component type identifier (string), and the
   * corresponding value is a function that takes a JSON object and deserializes it
   * into the appropriate component type.
   *
   * @details The functions stored in this map are typically used during deserialization
   * processes where JSON data needs to be converted back into component objects within
   * an entity-component system or similar architecture.
   */
  std::unordered_map<std::string, std::function<void( const nlohmann::json & )>> m_component_loaders;

  /**
   * @brief Initializes the component registry for the persistent system.
   *
   * This function sets up and registers all necessary components that need to
   * persist across different game states or scenes. It should be called during
   * system initialization to ensure all persistent components are properly
   * configured and available for use.
   */
  void initializeComponentRegistry();

  /**
   * @brief Registers a component type with a corresponding loader function.
   *
   * This template function allows the registration of a component type by associating
   * it with a unique key (string) and a loader function that handles json serialization/deserialization.
   * The loader function is stored in the `m_component_loaders` map.
   *
   * @note Components registered here will have their value loaded from/saved to
   * `res/json/persistent_components.json`
   *
   * @tparam ComponentType The type of the component to register.
   * @param key A unique string identifier for the corresponding json object.
   */
  template <typename ComponentType> void registerComponent( const std::string &key )
  {
    m_component_loaders[key] = [this]( const nlohmann::json &value ) {
      auto &component = get_persistent_component<ComponentType>();
      component.deserialize( value );
      auto deserialized_value = component();
      SPDLOG_INFO( "Loaded {} from JSON with value {}", component.class_name(), deserialized_value );
    };
  }
};

} // namespace ProceduralMaze::Sys

#endif // _INC_PERSISTENT_SYSTEM_HPP