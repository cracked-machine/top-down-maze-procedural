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
  BaseSystem( ProceduralMaze::SharedEnttRegistry reg )
      : m_reg( reg )
  {
  }

  ~BaseSystem() = default;

  template <typename T> void add_persistent_component( entt::registry &reg )
  {
    if ( not reg.ctx().contains<T>() ) { reg.ctx().emplace<T>(); }
  }

  // Get a grid position from an entity's Position component
  std::optional<sf::Vector2i> getGridPosition( entt::entity entity ) const
  {
    auto pos = m_reg->try_get<Cmp::Position>( entity );
    if ( pos )
    {
      return std::optional<sf::Vector2i>{
          { static_cast<int>( pos->x / Sprites::MultiSprite::kDefaultSpriteDimensions.x ),
            static_cast<int>( pos->y / Sprites::MultiSprite::kDefaultSpriteDimensions.y ) } };
    }
    return std::nullopt;
  }

  // Get a pixel position from an entity's Position component
  std::optional<sf::Vector2f> getPixelPosition( entt::entity entity ) const
  {
    auto pos = m_reg->try_get<Cmp::Position>( entity );
    if ( pos ) { return *pos; }
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

  sf::FloatRect get_hitbox( sf::Vector2f pos )
  {
    return sf::FloatRect( { pos.x, pos.y }, sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions } );
  }

  sf::Vector2f snap_to_grid( const sf::Vector2f &position )
  {
    float grid_size = Sprites::MultiSprite::kDefaultSpriteDimensions.x; // Assuming square grid

    return sf::Vector2f( std::round( position.x / grid_size ) * grid_size,
                         std::round( position.y / grid_size ) * grid_size );
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
  bool is_valid_move( sf::Vector2f &player_position );

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
  bool isDiagonalMovementBetweenObstacles( const sf::Vector2f &current_pos, const sf::Vector2f &direction );

  // The game display resolution in pixels
  static constexpr sf::Vector2u kDisplaySize{ 1920, 1024 };

  // The playable area size in blocks, not pixels
  static constexpr sf::Vector2u kMapGridSize{ 100u, 62u };
  // The playable area offset in blocks, not pixels
  static constexpr sf::Vector2f kMapGridOffset{ 10.f, 1.f };

  // singleton event dispatcher
  static entt::dispatcher &getEventDispatcher()
  {
    if ( !m_event_dispatcher ) { m_event_dispatcher = std::make_unique<entt::dispatcher>(); }
    return *m_event_dispatcher;
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