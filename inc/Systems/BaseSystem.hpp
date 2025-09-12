#ifndef __SYSTEMS_BASE_SYSTEM_HPP__
#define __SYSTEMS_BASE_SYSTEM_HPP__

#include <Position.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

#include <entt/signal/dispatcher.hpp>

namespace ProceduralMaze::Sys {

class BaseSystem
{
public:
  BaseSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg ) : m_reg( reg ) {}

  ~BaseSystem() = default;

  // Get a grid position from an entity's Position component
  std::optional<sf::Vector2i> getGridPosition( entt::entity entity ) const
  {
    auto pos = m_reg->try_get<Cmp::Position>( entity );
    if ( pos )
    {
      return std::optional<sf::Vector2i>{
          { static_cast<int>( pos->x / Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.x ),
            static_cast<int>( pos->y / Sprites::MultiSprite::DEFAULT_SPRITE_SIZE.y ) }
      };
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
  float getManhattanDistance( sf::Vector2f posA, sf::Vector2f posB ) const { return std::abs( posA.x - posB.x ) + std::abs( posA.y - posB.y ); }

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
    return sf::FloatRect( { pos.x, pos.y }, sf::Vector2f{ Sprites::MultiSprite::DEFAULT_SPRITE_SIZE } );
  }

  constexpr static const sf::Vector2u DISPLAY_SIZE{ 1920, 1024 };

  // MAP_GRID_OFFSET and MAP_GRID_SIZE are in blocks, not pixels
  const sf::Vector2f MAP_GRID_OFFSET{ 10.f, 1.f };
  const sf::Vector2u MAP_GRID_SIZE{ 100u, 61u };

  // singleton event dispatcher
  static entt::dispatcher &getEventDispatcher()
  {
    if ( !m_event_dispatcher ) { m_event_dispatcher = std::make_unique<entt::dispatcher>(); }
    return *m_event_dispatcher;
  }

protected:
  // Entity registry
  std::shared_ptr<entt::basic_registry<entt::entity>> m_reg;
  sf::Vector2f PLAYER_START_POS{ 20, static_cast<float>( DISPLAY_SIZE.y ) / 2 };

private:
  // Prevent access to uninitialised dispatcher - use getEventDispatcher()
  static std::unique_ptr<entt::dispatcher> m_event_dispatcher;
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_BASE_SYSTEM_HPP__