#ifndef SRC_SYSTEMS_BASESYSTEM_HPP__
#define SRC_SYSTEMS_BASESYSTEM_HPP__

#include <Audio/SoundBank.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>

#include <functional>
#include <spdlog/spdlog.h>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Loot.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze
{
// using SharedEnttRegistry = std::shared_ptr<entt::basic_registry<entt::entity>>;

namespace Sys
{

class BaseSystem
{
public:
  // The size of each grid square in pixels
  inline static constexpr sf::Vector2u kGridSquareSizePixels{ 16u, 16u };
  inline static constexpr sf::Vector2f kGridSquareSizePixelsF{ 16.f, 16.f };

  // The game display resolution in pixels
  inline static constexpr sf::Vector2u kDisplaySize{ 1920, 1024 };

  // The playable area size in blocks, not pixels
  inline static constexpr sf::Vector2u kMapGridSize{ 100u, 124u };

  // The playable area offset in blocks, not pixels
  inline static constexpr sf::Vector2f kMapGridOffset{ 1.f, 1.f };

  BaseSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief polymorphic destructor for derived classes
  virtual ~BaseSystem() { SPDLOG_INFO( "BaseSystem destructor called for system at {}", static_cast<void *>( this ) ); };

  //! @brief Event handler for pausing system clocks. Must be implemented by derived classes.
  //! @note If you register this handler with the event dispatcher, this function is automcatically called when the game is paused.
  /// For example:
  /// `std::ignore = getEventDispatcher().sink<Events::PauseClocksEvent>().connect<&Sys::DerivedSystem::onPause>(this);`
  virtual void onPause() = 0;

  //! @brief Event handler for resuming system clocks. Must be implemented by derived classes.
  //! @note If you register this handler with the event dispatcher, this function is automcatically called when the game is resumed.
  /// For example:
  /// `std::ignore = getEventDispatcher().sink<Events::ResumeClocksEvent>().connect<&Sys::DerivedSystem::onResume>(this);`
  virtual void onResume() = 0;

  //! @brief Add a persistent component to the registry's context if it doesn't already exist
  //!
  //! @tparam T
  template <typename T>
  void add_persistent_component()
  {
    if ( not getReg().ctx().contains<T>() ) { getReg().ctx().emplace<T>(); }
  }

  //! @brief Add a persistent component with constructor arguments
  //!
  //! @tparam T
  //! @tparam Args
  //! @param args
  template <typename T, typename... Args>
  void add_persistent_component( Args &&...args )
  {
    if ( not getReg().ctx().contains<T>() ) { getReg().ctx().emplace<T>( std::forward<Args>( args )... ); }
  }

  //! @brief Get the persistent component object
  //!
  //! @tparam T
  //! @return T&
  template <typename T>
  T &get_persistent_component()
  {
    if ( not getReg().ctx().contains<T>() )
    {
      SPDLOG_CRITICAL( "Attempting to access non-existent persistent component: {}", typeid( T ).name() );
      throw std::runtime_error( "Persistent component not found: " + std::string( typeid( T ).name() ) );
    }
    return getReg().ctx().get<T>();
  }

  //! @brief Get the persistent component object
  //!
  //! @tparam T
  //! @return T&
  template <typename T>
  static T &get_persistent_component( entt::registry &reg )
  {
    if ( not reg.ctx().contains<T>() )
    {
      SPDLOG_CRITICAL( "Attempting to access non-existent persistent component: {}", typeid( T ).name() );
      throw std::runtime_error( "Persistent component not found: " + std::string( typeid( T ).name() ) );
    }
    return reg.ctx().get<T>();
  }

  //! @brief Get the Grid Position object
  //!
  //! @param entity The entity to get the grid position for.
  //! @return std::optional<sf::Vector2i>
  std::optional<sf::Vector2i> getGridPosition( entt::entity entity ) const
  {
    auto pos = getReg().try_get<Cmp::Position>( entity );

    if ( pos )
    {
      return std::optional<sf::Vector2i>{ { static_cast<int>( pos->position.x / BaseSystem::kGridSquareSizePixels.x ),
                                            static_cast<int>( pos->position.y / BaseSystem::kGridSquareSizePixels.y ) } };
    }
    else { SPDLOG_ERROR( "Entity {} does not have a Position component", static_cast<int>( entity ) ); }
    return std::nullopt;
  }

  //! @brief Get the Pixel Position object from an entity's Position component.
  //!
  //! @param entity The entity to get the pixel position for.
  //! @return std::optional<sf::Vector2f>
  std::optional<sf::Vector2f> getPixelPosition( entt::entity entity ) const
  {
    auto pos = getReg().try_get<Cmp::Position>( entity );

    if ( pos ) { return ( *pos ).position; }
    return std::nullopt;
  }

  //! @brief Checks if the player's movement to a given position is valid
  //! Validates whether the player can move to the specified position by checking
  //! for collisions with walls, boundaries, or other obstacles in the game world.
  //! @param player_position The target position to validate for player movement
  //! @return true if the movement is valid and allowed, false otherwise
  bool is_valid_move( const sf::FloatRect &player_position );

  //! @brief Checks if a diagonal movement would pass between two obstacles.
  //! This function determines whether a diagonal movement from the current position
  //! in the specified direction would result in the player squeezing between two
  //! obstacles (e.g., moving diagonally through a corner where two walls meet).
  //! This is typically used to prevent unrealistic movement through tight spaces.
  //!
  //! @param current_pos The current position of the player as a 2D vector
  //! @param direction The direction vector representing the intended diagonal movement
  //! @return true if the diagonal movement would pass between obstacles, false otherwise
  bool isDiagonalMovementBetweenObstacles( const sf::FloatRect &current_pos, const sf::Vector2f &direction );

  // singleton event dispatcher
  // Use this to get temporary access to the dispatcher to register event handlers
  static entt::dispatcher &get_systems_event_queue()
  {
    if ( !m_systems_event_queue ) { m_systems_event_queue = std::make_unique<entt::dispatcher>(); }
    return *m_systems_event_queue;
  }

  // Helper structs for variadic template parameter packs
  template <typename... Types>
  struct IncludePack
  {
  };

  // Helper structs for variadic template parameter packs
  template <typename... Types>
  struct ExcludePack
  {
  };

  /**
   * @brief Retrieves a random entity and its position component from entities matching the specified criteria.
   *
   * This function selects a random entity and position component from a filtered view of entities that have a Position
   * component and all specified Include components, while excluding entities with any of the Exclude components.
   *
   * @tparam Include... Variadic template parameter pack specifying component types that entities must have
   * @tparam Exclude... Variadic template parameter pack specifying component types that entities must not have
   *
   * @param include_pack Template parameter pack wrapper for components to include in the filter. Do not include
   * Cmp::Position here.
   * @param exclude_pack Template parameter pack wrapper for components to exclude from the filter
   * @param seed Optional seed value for random number generation. If 0 (default), uses std::random_device
   *
   * @return std::pair<entt::entity, Cmp::Position> A pair containing the randomly selected entity and its position
   * component
   *
   * @note The function assumes there is at least one entity matching the filter criteria.
   *       If no entities match, the behavior is undefined.
   * @note Uses SPDLOG_DEBUG to log the number of matching positions found.
   */
  template <typename... Include, typename... Exclude>
  std::pair<entt::entity, Cmp::Position> get_random_position( IncludePack<Include...>, ExcludePack<Exclude...>, unsigned long seed = 0 )
  {
    auto random_view = getReg().view<Cmp::Position, Include...>( entt::exclude<Exclude...> );

    auto random_view_count = std::distance( random_view.begin(), random_view.end() );

    SPDLOG_DEBUG( "Found {} positions in the maze.", random_view_count );

    // Get random index and advance iterator to that position
    Cmp::RandomInt seed_picker( 0, static_cast<int>( random_view_count - 1 ) );
    if ( seed != 0 ) { seed_picker.seed( seed ); }
    else { seed_picker.seed( std::random_device{}() ); }

    int random_index = seed_picker.gen();
    SPDLOG_DEBUG( "Random index selected: {}", random_index );
    if ( random_index < 0 || random_index >= random_view_count )
    {
      SPDLOG_CRITICAL( "Random index {} out of bounds (0 to {})", random_index, random_view_count - 1 );
      throw std::out_of_range( "Random index out of bounds" );
    }
    auto it = random_view.begin();
    std::advance( it, random_index );

    // Get the random entity
    entt::entity random_entity = *it;

    // Get the position component
    Cmp::Position random_position = random_view.template get<Cmp::Position>( random_entity );

    return { random_entity, random_position };
  }

  template <typename... Include, typename... Exclude>
  entt::entity get_random_nearby_disabled_obstacle( sf::FloatRect search_area, IncludePack<Include...>, ExcludePack<Exclude...> )
  {

    auto obst_view = getReg().view<Cmp::Obstacle, Cmp::Position, Include...>( entt::exclude<Exclude...> );

    for ( auto obst_entity : obst_view )
    {
      auto &obst_cmp = obst_view.template get<Cmp::Obstacle>( obst_entity );
      auto &obst_pos_cmp = obst_view.template get<Cmp::Position>( obst_entity );
      if ( obst_cmp.m_enabled ) continue; // only drop the loot at disabled (traversable) obstacle
      if ( not search_area.findIntersection( obst_pos_cmp ) ) continue;

      return obst_entity;
    }
    SPDLOG_WARN( "Failed to find nearby disabled obstacle for [{},{}].", search_area.position.x, search_area.position.y );
    return entt::null;
  }

  entt::registry &getReg() { return m_reg.get(); }
  const entt::registry &getReg() const { return m_reg.get(); }
  void setReg( entt::registry &reg ) { m_reg = std::ref( reg ); }

protected:
  // Entity registry: non-owning, re-assignable reference (by SceneManager)
  // The registry is owned by the current Scene.
  std::reference_wrapper<entt::registry> m_reg;

  //! @brief Non-owning reference to the OpenGL window
  sf::RenderWindow &m_window;

  //! @brief Non-owning reference to the shared sprite factory
  Sprites::SpriteFactory &m_sprite_factory;

  //! @brief Non-owning reference to the shared sound effects system
  Audio::SoundBank &m_sound_bank;

private:
  // Prevent access to uninitialised dispatcher - use getEventDispatcher()
  static std::unique_ptr<entt::dispatcher> m_systems_event_queue;
};

} // namespace Sys
} // namespace ProceduralMaze

#endif // SRC_SYSTEMS_BASESYSTEM_HPP__