#ifndef __SPRITES_TILEMAP_HPP__
#define __SPRITES_TILEMAP_HPP__

#include <Random.hpp>
#include <SFML/Graphics.hpp>
#include <exception>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sprites::Containers {

class TileMap : public sf::Drawable, public sf::Transformable
{
public:
  TileMap()
  {

    for ( int x = 0; x < 200; x++ )
      for ( int y = 0; y < 98; y++ )
        m_floortile_choices.push_back( FLOOR_TILE_POOL[m_floortile_picker.gen()] );

    if ( !load( m_tile_file.string(), { 16, 16 }, m_floortile_choices.data(), 200, 98 ) )
    {
      SPDLOG_CRITICAL( "Unable to load tile map {}", m_tile_file.string() );
      std::terminate();
    }
  }
  bool load( const std::filesystem::path &tileset, sf::Vector2u tileSize, const unsigned int *tiles, unsigned int width, unsigned int height )
  {
    // load the tileset texture
    if ( !m_tileset.loadFromFile( tileset ) ) return false;

    // resize the vertex array to fit the level size
    m_vertices.setPrimitiveType( sf::PrimitiveType::Triangles );
    m_vertices.resize( width * height * 6 );

    // populate the vertex array, with two triangles per tile
    for ( unsigned int i = 0; i < width; ++i )
    {
      for ( unsigned int j = 0; j < height; ++j )
      {
        // get the current tile number
        const int tileNumber = tiles[i + j * width];

        // find its position in the tileset texture
        const int tu = tileNumber % ( m_tileset.getSize().x / tileSize.x );
        const int tv = tileNumber / ( m_tileset.getSize().x / tileSize.x );

        // get a pointer to the triangles' vertices of the current tile
        sf::Vertex *triangles = &m_vertices[( i + j * width ) * 6];

        // define the 6 corners of the two triangles
        triangles[0].position = sf::Vector2f( i * tileSize.x, j * tileSize.y );
        triangles[1].position = sf::Vector2f( ( i + 1 ) * tileSize.x, j * tileSize.y );
        triangles[2].position = sf::Vector2f( i * tileSize.x, ( j + 1 ) * tileSize.y );
        triangles[3].position = sf::Vector2f( i * tileSize.x, ( j + 1 ) * tileSize.y );
        triangles[4].position = sf::Vector2f( ( i + 1 ) * tileSize.x, j * tileSize.y );
        triangles[5].position = sf::Vector2f( ( i + 1 ) * tileSize.x, ( j + 1 ) * tileSize.y );

        // define the 6 matching texture coordinates
        triangles[0].texCoords = sf::Vector2f( tu * tileSize.x, tv * tileSize.y );
        triangles[1].texCoords = sf::Vector2f( ( tu + 1 ) * tileSize.x, tv * tileSize.y );
        triangles[2].texCoords = sf::Vector2f( tu * tileSize.x, ( tv + 1 ) * tileSize.y );
        triangles[3].texCoords = sf::Vector2f( tu * tileSize.x, ( tv + 1 ) * tileSize.y );
        triangles[4].texCoords = sf::Vector2f( ( tu + 1 ) * tileSize.x, tv * tileSize.y );
        triangles[5].texCoords = sf::Vector2f( ( tu + 1 ) * tileSize.x, ( tv + 1 ) * tileSize.y );
      }
    }

    return true;
  }

private:
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override
  {
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = &m_tileset;

    // draw the vertex array
    target.draw( m_vertices, states );
  }

  sf::VertexArray m_vertices;
  sf::Texture m_tileset;

  const std::vector<unsigned int> FLOOR_TILE_POOL{ // res/Pixel Lands Dungeons/objects.png
                                                   48, 48, 48, 48, 48, 48, 49
  };

  Cmp::Random m_floortile_picker{ 0, static_cast<int>( FLOOR_TILE_POOL.size() - 1 ) };

  std::vector<uint32_t> m_floortile_choices;

  std::filesystem::path m_tile_file{ "res/kenney_tiny-dungeon/Tilemap/tilemap_packed.png" };
};

} // namespace ProceduralMaze::Sprites::Containers

#endif // __SPRITES_TILEMAP_HPP__