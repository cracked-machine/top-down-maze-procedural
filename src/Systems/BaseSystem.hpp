#ifndef __SYSTEMS_BASE_SYSTEM_HPP__
#define __SYSTEMS_BASE_SYSTEM_HPP__

#include <Audio/SoundBank.hpp>
#include <Components/PlayableCharacter.hpp>
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

  //! @brief Get the Manhattan Distance between two positions.
  //! Creates a grid-like distance metric:
  //! ┌────┬────┬────┐
  //! │ 2  │ 1  │ 2  │  Sum of absolute differences
  //! ├────┼────┼────┤  Moves only horizontal/vertical
  //! │ 1  │ 0  │ 1  │  (Like a taxi in a city grid)
  //! ├────┼────┼────┤
  //! │ 2  │ 1  │ 2  │
  //! └────┴────┴────┘
  //! @note NPCs will be unable to "see" around corners with this distance metric.
  //! @param posA The first position.
  //! @param posB The second position.
  //! @return unsigned int The Manhattan distance.
  template <typename T>
  T getManhattanDistance( sf::Vector2<T> posA, sf::Vector2<T> posB ) const
  {
    return std::abs( posA.x - posB.x ) + std::abs( posA.y - posB.y );
  }

  //! @brief Get the Chebyshev Distance between two positions.
  //! Creates an equal-cost distance metric for all 8 directions:
  //! ┌────┬────┬────┐
  //! │ 1  │ 1  │ 1  │  Maximum of x or y distance
  //! ├────┼────┼────┤  All 8 neighbors are distance 1
  //! │ 1  │ 0  │ 1  │  (Like a chess king's move)
  //! ├────┼────┼────┤
  //! │ 1  │ 1  │ 1  │
  //! └────┴────┴────┘
  //! @note NPC pathfinding will pick randomly and appear to zig-zag
  //! @param posA The first position.
  //! @param posB The second position.
  //! @return unsigned int The Chebyshev distance.
  template <typename T>
  T getChebyshevDistance( sf::Vector2<T> posA, sf::Vector2<T> posB ) const
  {
    return std::max( std::abs( posA.x - posB.x ), std::abs( posA.y - posB.y ) );
  }

  //! @brief Get the Euclidean Distance between two positions.
  //! Creates a straight-line distance metric:
  //! ┌─────┬─────┬─────┐
  //! │ 1.4 │ 1.0 │ 1.4 │  Straight-line distance
  //! ├─────┼─────┼─────┤  Diagonal = √2 ≈ 1.414
  //! │ 1.0 │ 0.0 │ 1.0 │  (Standard geometric distance)
  //! ├─────┼─────┼─────┤
  //! │ 1.4 │ 1.0 │ 1.4 │
  //! └─────┴─────┴─────┘
  //! @note NPCs pathfinding will be able to navigate around obstacles
  //! @param posA The first position.
  //! @param posB The second position.
  //! @return unsigned int The Euclidean distance.
  template <typename T>
  T getEuclideanDistance( sf::Vector2<T> posA, sf::Vector2<T> posB ) const
  {
    T dx = posA.x - posB.x;
    T dy = posA.y - posB.y;
    return static_cast<T>( std::sqrt( dx * dx + dy * dy ) );
  }

  //! @brief Snaps a rectangle's position to the nearest grid cell.
  //!
  //! Computes a new rectangle whose top-left corner is moved to the nearest grid
  //! intersection based on BaseSystem::kGridSquareSizePixels.x (the function
  //! assumes a square grid and uses the x component as the grid cell size).
  //! The input rectangle's size is preserved.
  //!
  //! Rounding uses std::round, so coordinates are mapped to the nearest multiple
  //! of the grid size (negative coordinates are handled correctly by std::round).
  //!
  //! @param position The rectangle to be snapped. Only its position is considered;
  //!                 the rectangle's size remains unchanged.
  //! @return A new sf::FloatRect with the snapped position and the original size.
  //!
  //! @note If the grid is not square, only the x component of
  //!       BaseSystem::kGridSquareSizePixels is used for both axes.
  sf::FloatRect snap_to_grid( const sf::FloatRect &position )
  {
    float grid_size = BaseSystem::kGridSquareSizePixels.x; // Assuming square grid
    sf::Vector2f snapped_pos{ std::round( position.position.x / BaseSystem::kGridSquareSizePixels.x ) * grid_size,
                              std::round( position.position.y / BaseSystem::kGridSquareSizePixels.y ) * grid_size };

    return sf::FloatRect( snapped_pos, position.size );
  }

  //! @brief Snap a given position to the nearest grid square.
  //! This function takes a 2D position and rounds its coordinates to the nearest
  //! grid square based on the grid size defined in BaseSystem::kGridSquareSizePixels.
  //! It assumes the grid squares are of uniform size.
  //!
  //! @param position The position to snap, as an sf::Vector2f.
  //! @return sf::Vector2f The snapped position aligned to the grid.
  sf::Vector2f snap_to_grid( const sf::Vector2f &position )
  {
    float grid_size = BaseSystem::kGridSquareSizePixels.x; // Assuming square grid
    sf::Vector2f snapped_pos{ std::round( position.x / BaseSystem::kGridSquareSizePixels.x ) * grid_size,
                              std::round( position.y / BaseSystem::kGridSquareSizePixels.y ) * grid_size };

    return snapped_pos;
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

  /**
   * @brief Calculates the bounding rectangle of the given SFML view.
   *
   * This function computes the world-space bounds represented by the specified sf::View.
   * The returned sf::FloatRect describes the area visible through the view.
   *
   * @param view Reference to the SFML view whose bounds are to be calculated.
   * @return sf::FloatRect The rectangle representing the view's bounds in world coordinates.
   */
  sf::FloatRect calculate_view_bounds( const sf::View &view ) const;

  /**
   * @brief Determines if a given position rectangle is visible within the specified view bounds.
   *
   * This function checks whether the provided position rectangle intersects with the view bounds,
   * indicating that the position is at least partially visible in the current view.
   *
   * @param viewbounds The rectangle representing the bounds of the current view.
   * @param position The rectangle representing the position to check for visibility.
   * @return true if the position is visible within the view bounds; false otherwise.
   */
  bool is_visible_in_view( const sf::FloatRect &viewbounds, const sf::FloatRect &position ) const;

  /**
   * @brief Checks if a position is visible within a given view's bounds
   *
   * @param view The view to check against (in world coordinates)
   * @param position The position to test for visibility
   * @return true if the position's hitbox intersects with the view bounds
   */
  bool is_visible_in_view( const sf::View &view, const sf::FloatRect &position ) const;

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
  std::pair<entt::entity, Cmp::Position> get_random_position( IncludePack<Include...>, ExcludePack<Exclude...>,
                                                              unsigned long seed = 0 )
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

  /**
   * @brief Attempts to create a loot drop at a traversable obstacle within a specified search area.
   *
   * Iterates over obstacles in the registry that match the given inclusion and exclusion criteria.
   * For each obstacle, checks if its position intersects with the search area and if it is disabled (traversable).
   * If a suitable obstacle is found, creates a new entity, assigns the provided loot component to it,
   * and places it at the obstacle's position.
   *
   * @tparam Include... Component types to include in the view.
   * @tparam Exclude... Component types to exclude from the view.
   * @param loot_cmp The loot component to be assigned to the new entity.
   * @param search The area in which to search for a suitable obstacle.
   * @param include_pack Pack of components to include in the view.
   * @param exclude_pack Pack of components to exclude from the view.
   * @return entt::entity The newly created loot entity, or entt::null if no suitable location was found.
   */
  template <typename... Include, typename... Exclude>
  entt::entity create_loot_drop( Cmp::Loot &&loot_cmp, sf::FloatRect search, IncludePack<Include...>, ExcludePack<Exclude...> )
  {
    auto obst_view = getReg().view<Cmp::Obstacle, Cmp::Position, Include...>( entt::exclude<Exclude...> );

    for ( auto [obst_entity, obst_cmp, obst_pos_cmp] : obst_view.each() )
    {
      if ( not search.findIntersection( obst_pos_cmp ) ) continue;
      if ( obst_cmp.m_enabled ) continue; // only drop the loot at disabled (traversable) obstacle

      auto loot_entity = getReg().create();
      getReg().emplace<Cmp::Loot>( loot_entity, std::move( loot_cmp ) );
      getReg().emplace_or_replace<Cmp::Position>( loot_entity, obst_pos_cmp );
      return loot_entity;
    }
    SPDLOG_WARN( "Failed to drop {} at [{},{}].", loot_cmp.m_type, search.position.x, search.position.y );
    return entt::null;
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

#endif // __SYSTEMS_BASE_SYSTEM_HPP__