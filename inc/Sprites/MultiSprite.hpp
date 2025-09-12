#ifndef __SPRITES_MULTISPRITE2_HPP__
#define __SPRITES_MULTISPRITE2_HPP__

// #include <Components/Random.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace ProceduralMaze::Sprites {

class MultiSprite : public sf::Drawable, public sf::Transformable
{
public:
  // Used by SpriteFactory::SpriteMetaData struct declaration
  MultiSprite() = default;

  /**
   * @brief Constructs a MultiSprite with a single tilemap.
   *
   * @param tilemap_path Path to the tilemap image file
   * @param tilemap_picks Vector of tile indices to extract from the tilemap
   * @param tileSize Size of each individual tile in pixels (default: 16x16)
   */
  MultiSprite( const std::filesystem::path &tilemap_path, std::vector<uint32_t> tilemap_picks ) { add_sprite( tilemap_path, tilemap_picks ); }

  void add_sprite( const std::filesystem::path &tilemap_path, std::vector<uint32_t> tilemap_picks )
  {
    if ( !m_tilemap_texture.loadFromFile( tilemap_path ) )
    {
      SPDLOG_CRITICAL( "Unable to load tile map {}", tilemap_path.string() );
      std::terminate();
    }

    SPDLOG_DEBUG( "Processing multisprite for {}", name );

    for ( const auto &tile_idx : tilemap_picks )
    {

      sf::VertexArray current_va( sf::PrimitiveType::Triangles, 6 );
      const int tu = tile_idx % ( m_tilemap_texture.getSize().x / DEFAULT_SPRITE_SIZE.x );
      const int tv = tile_idx / ( m_tilemap_texture.getSize().x / DEFAULT_SPRITE_SIZE.x );

      // draw the two triangles within local space using the `DEFAULT_SPRITE_SIZE`
      current_va[0].position = sf::Vector2f( 0, 0 );
      current_va[1].position = sf::Vector2f( DEFAULT_SPRITE_SIZE.x, 0 );
      current_va[2].position = sf::Vector2f( 0, DEFAULT_SPRITE_SIZE.y );
      current_va[3].position = sf::Vector2f( 0, DEFAULT_SPRITE_SIZE.y );
      current_va[4].position = sf::Vector2f( DEFAULT_SPRITE_SIZE.x, 0 );
      current_va[5].position = sf::Vector2f( DEFAULT_SPRITE_SIZE.x, DEFAULT_SPRITE_SIZE.y );

      current_va[0].texCoords = sf::Vector2f( tu * DEFAULT_SPRITE_SIZE.x, tv * DEFAULT_SPRITE_SIZE.y );
      current_va[1].texCoords = sf::Vector2f( ( tu + 1 ) * DEFAULT_SPRITE_SIZE.x, tv * DEFAULT_SPRITE_SIZE.y );
      current_va[2].texCoords = sf::Vector2f( tu * DEFAULT_SPRITE_SIZE.x, ( tv + 1 ) * DEFAULT_SPRITE_SIZE.y );
      current_va[3].texCoords = sf::Vector2f( tu * DEFAULT_SPRITE_SIZE.x, ( tv + 1 ) * DEFAULT_SPRITE_SIZE.y );
      current_va[4].texCoords = sf::Vector2f( ( tu + 1 ) * DEFAULT_SPRITE_SIZE.x, tv * DEFAULT_SPRITE_SIZE.y );
      current_va[5].texCoords = sf::Vector2f( ( tu + 1 ) * DEFAULT_SPRITE_SIZE.x, ( tv + 1 ) * DEFAULT_SPRITE_SIZE.y );
      SPDLOG_TRACE( "  - Added tile index {} (tu={},tv={})", tile_idx, tu, tv );

      m_va_list.push_back( current_va );
    }
    SPDLOG_DEBUG(
        "Requested {} tiles ... Created {} sprites from texture {}: ",
        std::distance( tilemap_picks.begin(), std::unique( tilemap_picks.begin(), tilemap_picks.end() ) ), m_va_list.size(), tilemap_path.string()
    );
  }

  void pick( unsigned int idx, std::string caller )
  {
    if ( m_va_list.empty() )
    {
      SPDLOG_WARN( "pick() called on empty sprite list" );
      return;
    }

    if ( idx > m_va_list.size() - 1 )
    {
      m_selected_vertices = m_va_list[0];
      SPDLOG_WARN(
          "{}: pick() index {} out of range, selecting index 0 "
          "instead. m_va_list size is {}",
          caller, idx, m_va_list.size()
      );
    }
    else { m_selected_vertices = m_va_list[idx]; }
  }

  sf::Texture m_tilemap_texture;

  std::size_t get_sprite_count() const { return m_va_list.size(); }
  static const sf::Vector2u DEFAULT_SPRITE_SIZE;

private:
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override
  {
    if ( m_selected_vertices.getVertexCount() == 0 )
    {
      SPDLOG_CRITICAL( "No vertex array selected. Use pick() to select one." );
      std::terminate();
    }
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = &m_tilemap_texture;

    // draw the vertex array
    target.draw( m_selected_vertices, states );
  }

  std::vector<sf::VertexArray> m_va_list;
  // select the first vertex array by default
  sf::VertexArray m_selected_vertices;
};

} // namespace ProceduralMaze::Sprites

#endif // __SPRITES_MULTISPRITE2_HPP__