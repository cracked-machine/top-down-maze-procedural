#ifndef __SYSTEMS_BASE_SYSTEM_HPP__
#define __SYSTEMS_BASE_SYSTEM_HPP__

#include <MultiSprite.hpp>
#include <Obstacle.hpp>
#include <Position.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

#include <entt/signal/dispatcher.hpp>

namespace ProceduralMaze {
using SharedEnttRegistry = std::shared_ptr<entt::basic_registry<entt::entity>>;

namespace Sys {

class BaseSystem
{
public:
  BaseSystem( ProceduralMaze::SharedEnttRegistry reg );

  ~BaseSystem() = default;

  // The size of each grid square in pixels
  static const sf::Vector2u kGridSquareSizePixels;
  static const sf::Vector2f kGridSquareSizePixelsF;

  // The game display resolution in pixels
  static const sf::Vector2u kDisplaySize;

  // The playable area size in blocks, not pixels
  static const sf::Vector2u kMapGridSize;

  // The playable area offset in blocks, not pixels
  static const sf::Vector2f kMapGridOffset;

  // Add a persistent component to the registry's context if it doesn't already exist
  template <typename T> void add_persistent_component()
  {
    if ( not m_reg->ctx().contains<T>() ) { m_reg->ctx().emplace<T>(); }
  }

  // Add a persistent component with constructor arguments
  template <typename T, typename... Args> void add_persistent_component( Args &&...args )
  {
    if ( not m_reg->ctx().contains<T>() ) { m_reg->ctx().emplace<T>( std::forward<Args>( args )... ); }
  }

  // Get a persistent component from the registry's context
  template <typename T> T &get_persistent_component()
  {
    if ( not m_reg->ctx().contains<T>() )
    {
      SPDLOG_CRITICAL( "Attempting to access non-existent persistent component: {}", typeid( T ).name() );
      std::terminate();
    }
    return m_reg->ctx().get<T>();
  }

  // Get a grid position from an entity's Position component
  std::optional<sf::Vector2i> getGridPosition( entt::entity entity ) const
  {
    auto pos = m_reg->try_get<Cmp::Position>( entity );
    if ( pos )
    {
      return std::optional<sf::Vector2i>{
          { static_cast<int>( pos->position.x / BaseSystem::kGridSquareSizePixels.x ),
            static_cast<int>( pos->position.y / BaseSystem::kGridSquareSizePixels.y ) } };
    }
    return std::nullopt;
  }

  // Get a pixel position from an entity's Position component
  std::optional<sf::Vector2f> getPixelPosition( entt::entity entity ) const
  {
    auto pos = m_reg->try_get<Cmp::Position>( entity );
    if ( pos ) { return ( *pos ).position; }
    return std::nullopt;
  }

  // sum( (posA.x - posB.x) + (posA.y - posB.y) )
  // cardinal directions only
  unsigned int getManhattanDistance( sf::Vector2i posA, sf::Vector2i posB ) const
  {
    return std::abs( posA.x - posB.x ) + std::abs( posA.y - posB.y );
  }

  // sum( (posA.x - posB.x) + (posA.y - posB.y) )
  // cardinal directions only
  float getManhattanDistance( sf::Vector2f posA, sf::Vector2f posB ) const
  {
    return std::abs( posA.x - posB.x ) + std::abs( posA.y - posB.y );
  }

  // max( (posA.x - posB.x), (posA.y - posB.y) )
  // cardinal and diagonal directions
  unsigned int getChebyshevDistance( sf::Vector2i posA, sf::Vector2i posB ) const
  {
    return std::max( std::abs( posA.x - posB.x ), std::abs( posA.y - posB.y ) );
  }

  // max( (posA.x - posB.x), (posA.y - posB.y) )
  // cardinal and diagonal directions
  float getChebyshevDistance( sf::Vector2f posA, sf::Vector2f posB ) const
  {
    return std::max( std::abs( posA.x - posB.x ), std::abs( posA.y - posB.y ) );
  }

  // Snap a position to the nearest grid square
  sf::FloatRect snap_to_grid( const sf::FloatRect &position )
  {
    float grid_size = BaseSystem::kGridSquareSizePixels.x; // Assuming square grid
    sf::Vector2f snapped_pos{ std::round( position.position.x / BaseSystem::kGridSquareSizePixels.x ) * grid_size,
                              std::round( position.position.y / BaseSystem::kGridSquareSizePixels.y ) * grid_size };
    return sf::FloatRect( snapped_pos, position.size );
  }

  sf::Vector2f snap_to_grid( const sf::Vector2f &position )
  {
    float grid_size = BaseSystem::kGridSquareSizePixels.x; // Assuming square grid
    sf::Vector2f snapped_pos{ std::round( position.x / BaseSystem::kGridSquareSizePixels.x ) * grid_size,
                              std::round( position.y / BaseSystem::kGridSquareSizePixels.y ) * grid_size };
    return snapped_pos;
  }

  /**
   * @brief Checks if the player's movement to a given position is valid
   *
   * Validates whether the player can move to the specified position by checking
   * for collisions with walls, boundaries, or other obstacles in the game world.
   *
   * @param player_position The target position to validate for player movement
   * @return true if the movement is valid and allowed, false otherwise
   */
  bool is_valid_move( const sf::FloatRect &player_position );

  /**
   * @brief Checks if a diagonal movement would pass between two obstacles.
   *
   * This function determines whether a diagonal movement from the current position
   * in the specified direction would result in the player squeezing between two
   * obstacles (e.g., moving diagonally through a corner where two walls meet).
   * This is typically used to prevent unrealistic movement through tight spaces.
   *
   * @param current_pos The current position of the player as a 2D vector
   * @param direction The direction vector representing the intended diagonal movement
   * @return true if the diagonal movement would pass between obstacles, false otherwise
   */
  bool isDiagonalMovementBetweenObstacles( const sf::FloatRect &current_pos, const sf::Vector2f &direction );

  sf::FloatRect calculate_view_bounds( const sf::View &view ) const;
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
  static entt::dispatcher &getEventDispatcher()
  {
    if ( !m_event_dispatcher ) { m_event_dispatcher = std::make_unique<entt::dispatcher>(); }
    return *m_event_dispatcher;
  }

  // Helper structs for variadic template parameter packs
  template <typename... Types> struct IncludePack
  {
  };

  // Helper structs for variadic template parameter packs
  template <typename... Types> struct ExcludePack
  {
  };

  /**
   * @brief Retrieves a random entity and its position component from entities matching the specified criteria.
   *
   * This function selects a random entity from a filtered view of entities that have a Position component
   * and all specified Include components, while excluding entities with any of the Exclude components.
   *
   * @tparam Include... Variadic template parameter pack specifying component types that entities must have
   * @tparam Exclude... Variadic template parameter pack specifying component types that entities must not have
   *
   * @param include_pack Template parameter pack wrapper for components to include in the filter
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
    auto random_view = m_reg->view<Cmp::Position, Include...>( entt::exclude<Exclude...> );

    auto random_view_count = std::distance( random_view.begin(), random_view.end() );
    SPDLOG_DEBUG( "Found {} positions in the maze.", random_view_count );

    // Get random index and advance iterator to that position
    Cmp::RandomInt seed_picker( 0, static_cast<int>( random_view_count - 1 ) );
    if ( seed != 0 ) { seed_picker.seed( seed ); }
    else { seed_picker.seed( std::random_device{}() ); }

    int random_index = seed_picker.gen();
    auto it = random_view.begin();
    std::advance( it, random_index );

    // Get the random entity
    entt::entity random_entity = *it;

    // Get the position component
    Cmp::Position random_position = random_view.template get<Cmp::Position>( random_entity );

    return { random_entity, random_position };
  }

protected:
  // Entity registry
  ProceduralMaze::SharedEnttRegistry m_reg;

private:
  // Prevent access to uninitialised dispatcher - use getEventDispatcher()
  static std::unique_ptr<entt::dispatcher> m_event_dispatcher;
};

} // namespace Sys
} // namespace ProceduralMaze

#endif // __SYSTEMS_BASE_SYSTEM_HPP__