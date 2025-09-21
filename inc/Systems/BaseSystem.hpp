#ifndef __SYSTEMS_BASE_SYSTEM_HPP__
#define __SYSTEMS_BASE_SYSTEM_HPP__

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
  sf::Vector2f PLAYER_START_POS{ 20, static_cast<float>( kDisplaySize.y ) / 2 };

private:
  // Prevent access to uninitialised dispatcher - use getEventDispatcher()
  static std::unique_ptr<entt::dispatcher> m_event_dispatcher;
};

} // namespace Sys
} // namespace ProceduralMaze

#endif // __SYSTEMS_BASE_SYSTEM_HPP__