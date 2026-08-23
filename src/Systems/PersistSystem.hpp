#ifndef SRC_SYSTEMS_PERSISTSYSTEM_HPP__
#define SRC_SYSTEMS_PERSISTSYSTEM_HPP__

#include <Components/Persistent/IBasePersistent.hpp>
#include <Events/LoadSettingsEvent.hpp>
#include <Events/SaveSettingsEvent.hpp>
#include <Systems/BaseSystem.hpp>

#include <entt/entity/fwd.hpp>
#include <nlohmann/json_fwd.hpp>

#include <functional>
#include <unordered_map>

namespace Game::Sys
{

//! @brief Serializes and deserializes registered Cmp::Persist::IBasePersistent components to/from JSON,
//! so game settings and progress survive between sessions.
class PersistSystem : public BaseSystem
{
public:
  //! @brief Construct a new Persist System object
  //! @param reg
  //! @param window
  //! @param sprite_factory
  //! @param sound_bank
  PersistSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief Loads the persistent state data from storage.
  //! @details Retrieves previously saved state information and restores the system to its last
  //! known configuration. The state data typically includes settings, progress, or other
  //! persistent information that should survive between application sessions.
  //! @throws std::runtime_error if the state file cannot be read or is corrupted
  //! @throws std::ios_base::failure if file I/O operations fail
  void load_state();

  //! @brief Saves the current state data to persistent storage.
  //! @details Serializes the current configuration and state information of the system and
  //! writes it to a file or database, so that important settings and progress are preserved
  //! for future sessions.
  //! @throws std::ios_base::failure if file I/O operations fail
  void save_state();

  /// EVENTS
  //! @brief Event handler that forwards to save_state().
  //! @param event
  void on_save_settings_event( [[maybe_unused]] const Events::SaveSettingsEvent &event )
  {
    SPDLOG_DEBUG( "Save Settings Event received" );
    save_state();
  }

  //! @brief Event handler that forwards to load_state().
  //! @param event
  void on_load_settings_event( [[maybe_unused]] const Events::LoadSettingsEvent &event )
  {
    SPDLOG_DEBUG( "Load Settings Event received" );
    load_state();
  }

  //! @brief Initializes the component registry for the persistent system.
  //! @details Sets up and registers all necessary components that need to persist across
  //! different game states or scenes. Should be called during system initialization to ensure
  //! all persistent components are properly configured and available for use.
  void initialize_component_registry();

  // Only declare templates in public API
  //! @brief Add a persistent component to the registry's context if it doesn't already exist
  //!
  //! @tparam T
  //! @param reg
  template <typename T>
  static void add( entt::registry &reg );

  //! @brief Add a persistent component with constructor arguments
  //!
  //! @tparam T
  //! @tparam Args
  //! @param reg
  //! @param args
  template <typename T, typename... Args>
  static void add( entt::registry &reg, Args &&...args );

  //! @brief Get the persistent component object
  //!
  //! @tparam T
  //! @param reg
  //! @return T&
  template <typename T>
  static T &get( entt::registry &reg );

  //! @brief Accessor for RenderMenuSystem to iterate components
  const std::vector<Cmp::Persist::IBasePersistent *> &get_registered_components() { return m_registered_components; }

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  //! @brief Register the serialize/deserialize functions for every persistent component type.
  void initialize_type_registry();

  //! @brief registers the serialize/deserialize function with the given JSON object name and Component type.
  //! @tparam T
  //! @param name
  //! @param config
  template <typename T>
  void register_types( const std::string &name );

  //! @brief Map of component loader functions indexed by component type name.
  //! @details Each key is a component type identifier (string), and the corresponding value is
  //! a function that takes a JSON object and deserializes it into the appropriate component
  //! type. Used during deserialization to convert JSON data back into component objects.
  std::unordered_map<std::string, std::function<void( const nlohmann::json & )>> m_component_loaders;

  //! @brief Map of component deserialize functions indexed by component type name; populated by register_types().
  std::unordered_map<std::string, std::function<void( const nlohmann::json & )>> m_type_registry;

  //! @brief Map of component serialize functions indexed by component type name; populated by register_types().
  std::unordered_map<std::string, std::function<nlohmann::json()>> m_component_serializers;

  //! @brief Persistent components that implement Cmp::Persist::IBasePersistent, for RenderMenuSystem to iterate.
  std::vector<Cmp::Persist::IBasePersistent *> m_registered_components;
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_PERSISTSYSTEM_HPP__
