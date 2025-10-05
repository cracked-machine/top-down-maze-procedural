#ifndef __COMPONENTS_PLAYERSTARTPOSITION_HPP__
#define __COMPONENTS_PLAYERSTARTPOSITION_HPP__

#include <Base.hpp>
#include <BaseSystem.hpp>
#include <MultiSprite.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp::Persistent {

// Coordinate
class PlayerStartPosition : public sf::Vector2f
{
public:
  PlayerStartPosition( sf::Vector2f pos = sf::Vector2f( Sprites::MultiSprite::kDefaultSpriteDimensions.x * 2,
                                                        static_cast<float>( Sys::BaseSystem::kDisplaySize.y ) / 2 ) )
      : sf::Vector2f( pos )
  {
    SPDLOG_DEBUG( "PlayerStartPosition()" );
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __COMPONENTS_POSITION_HPP__