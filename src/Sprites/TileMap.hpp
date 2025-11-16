#ifndef __SPRITES_TILEMAP_HPP__
#define __SPRITES_TILEMAP_HPP__

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include <Components/Random.hpp>

namespace ProceduralMaze::Sprites::Containers
{

class TileMap : public sf::Drawable, public sf::Transformable
{
public:
  TileMap() = default;

  void load( sf::Vector2u map_dimensions, const std::filesystem::path &config_path = "res/json/tilemap_config.json" );

  // Create the tile map using a single large sf::VertexArray
  void create( sf::Vector2u tile_size, unsigned int width, unsigned int height );

  // Draw the sf::VertexArray to the render target (with optional state for shader)
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;

  void clear()
  {
    m_vertices.clear();
    m_floortile_choices.clear();
  }

private:
  struct TileMapConfig
  {
    std::filesystem::path texture_path;
    sf::Vector2u tile_size;
    sf::Vector2u map_dimensions;
    std::vector<unsigned int> floor_tile_pool;
    int random_seed = 0;
  };

  TileMapConfig load_config( const std::filesystem::path &config_path );

  void initialize( const TileMapConfig &config );

  sf::VertexArray m_vertices;
  sf::Texture m_tileset;
  std::vector<uint32_t> m_floortile_choices;
  sf::Clock m_clock{};
};

} // namespace ProceduralMaze::Sprites::Containers

#endif // __SPRITES_TILEMAP_HPP__