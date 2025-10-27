#ifndef __SPRITES_BASICSPRITE_HPP__
#define __SPRITES_BASICSPRITE_HPP__

#include <spdlog/spdlog.h>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Sprites {

class BasicSprite
{
public:
  BasicSprite( const std::string &texture_path )
  {
    if ( !m_texture.loadFromFile( texture_path ) ) { SPDLOG_CRITICAL( "Unable to load tile map {}", texture_path ); }
    m_texture.setSmooth( true );
  }

  sf::Sprite get_sprite( sf::Vector2f pos ) const
  {
    sf::Sprite sprite( m_texture );
    sprite.setPosition( pos );
    return sprite;
  }

protected:
  sf::Texture m_texture;
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_BASICSPRITE_HPP__