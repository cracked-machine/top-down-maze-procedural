#ifndef SRC_SYSTEMS_PERSISTENT_SYSTEM_HPP
#define SRC_SYSTEMS_PERSISTENT_SYSTEM_HPP

#include <Events/SaveSettingsEvent.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class PersistentSystem : public BaseSystem
{
public:
  PersistentSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

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
    SPDLOG_DEBUG( "Save Settings Event received" );
    save_state();
  }
  /**
   * @brief Initializes the component registry for the persistent system.
   *
   * This function sets up and registers all necessary components that need to
   * persist across different game states or scenes. It should be called during
   * system initialization to ensure all persistent components are properly
   * configured and available for use.
   */
  void initializeComponentRegistry();

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
   * @brief Registers a persistent component loader with default arguments.
   *
   * This function associates a component loader with a given key. The loader captures
   * the provided default arguments, constructing the initial component when invoked.
   * The initial default value is overridden by the deserialized JSON data.
   *
   * @tparam ComponentType The type of the persistent component to register.
   * @tparam DefaultArgTypes Variadic template parameter pack for default constructor arguments.
   * @param key The unique string identifier for the component loader.
   * @param default_args Default arguments to be forwarded to the component's constructor.
   *
   * @note The loader is stored internally and can be invoked later to create and
   *       initialize the component from a JSON value.
   */
  template <typename ComponentType, typename... DefaultArgTypes>
  void registerComponent( const std::string &key, DefaultArgTypes &&...default_args )
  {
    // Capture args in a tuple to preserve them for later use
    auto args_tuple = std::make_tuple( std::forward<DefaultArgTypes>( default_args )... );

    std::apply( [this]( auto &&...unpacked_args )
                { add_persistent_component<ComponentType>( std::forward<decltype( unpacked_args )>( unpacked_args )... ); }, args_tuple );

    // move the tuple into the lambda to avoid copies (pack copy forbidden by lambda)
    m_component_loaders[key] = [this, args_tuple = std::move( args_tuple )]( const nlohmann::json &persistent_object )
    {
      // Component already exists, just update it
      auto &component = get_persistent_component<ComponentType>();
      component.deserialize( persistent_object );
      auto deserialized_value = component.get_value();
      SPDLOG_DEBUG( "Loaded {} from JSON with value {}", component.class_name(), deserialized_value );
    };
  }
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PERSISTENT_SYSTEM_HPP