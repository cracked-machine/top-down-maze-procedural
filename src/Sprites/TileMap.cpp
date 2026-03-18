#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Constants.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <SceneControl/SceneConfig.hpp>
#include <Sprites/TileMap.hpp>

#include <SFML/Graphics/Color.hpp>
#include <entt/entity/registry.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <stdexcept>

namespace ProceduralMaze::Sprites::Containers
{

void TileMap::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
  if ( m_vertices.getVertexCount() == 0 )
  {
    SPDLOG_WARN( "Attempting to draw empty tilemap" );
    return;
  }

  // apply the transform
  states.transform *= getTransform();

  // apply the tileset texture
  states.texture = &m_tileset;

  // draw the vertex array
  target.draw( m_vertices, states );
}

void TileMap::create( const PathFinding::SpatialHashGrid &void_sm, const Scene::SceneConfigSharedPtr &sc )
{
  if ( not sc ) throw std::runtime_error( "Scene::SceneConfigSharedPtr is not initialised" );

  // load the tilemap from the scene configuration object
  try
  {
    if ( !std::filesystem::exists( sc->get_texture_path() ) )
    {
      SPDLOG_CRITICAL( "Texture file does not exist: {}", sc->get_texture_path().string() );
      throw std::runtime_error( "Texture file not found" );
    }
    if ( !m_tileset.loadFromFile( sc->get_texture_path() ) )
    {
      SPDLOG_CRITICAL( "Unable to load tileset texture file: {}", sc->get_texture_path().string() );
      throw std::runtime_error( "Failed to load texture" );
    }

  } catch ( const std::exception &e )
  {
    SPDLOG_CRITICAL( "Failed to initialize TileMap: {}", e.what() );
    throw;
  }

  // Validate texture is loaded before creating vertices
  if ( m_tileset.getSize().x == 0 || m_tileset.getSize().y == 0 )
  {
    SPDLOG_ERROR( "Tileset texture not loaded or invalid" );
    throw;
  }

  const sf::Vector2u texture_size = m_tileset.getSize();
  const unsigned int tiles_per_row = texture_size.x / sc->get_tile_size().x;

  Cmp::RandomInt floortile_picker{ 0, static_cast<int>( sc->get_floor_tile_pool().size() - 1 ) };
  // let json fix seed if specified as non-zero
  if ( sc->get_random_seed() != 0 )
  {
    floortile_picker.seed( static_cast<unsigned long>( sc->get_random_seed() ) );
    SPDLOG_DEBUG( "Using random seed: {}", sc->random_seed );
  }

  // resize the vertex array for squares made of two triangles
  m_vertices.setPrimitiveType( sf::PrimitiveType::Triangles );

  auto [map_size_grid, map_size_pixel] = sc->get_map_size();

  SPDLOG_INFO( "Generating tilemap for {}x{}", map_size_grid.x, map_size_grid.y );
  for ( unsigned int w = 0; w < map_size_grid.x; w++ )
  {
    for ( unsigned int h = 0; h < map_size_grid.y; h++ )
    {
      const unsigned int tile_number = sc->get_floor_tile_pool()[floortile_picker.gen()];

      // check if we have anything in the void spatial map for this position
      Cmp::Position lookup_position( { w * Constants::kGridSizePxF.x, h * Constants::kGridSizePxF.y }, Constants::kGridSizePxF );

      auto results = void_sm.at( lookup_position );
      if ( not results.empty() ) continue;

      // find its position in the tileset texture
      const unsigned int tu = tile_number % tiles_per_row;
      const unsigned int tv = tile_number / tiles_per_row;

      // Cache position calculations
      const float left = static_cast<float>( w * sc->get_tile_size().x );
      const float top = static_cast<float>( h * sc->get_tile_size().y );
      const float right = static_cast<float>( ( w + 1 ) * sc->get_tile_size().x );
      const float bottom = static_cast<float>( ( h + 1 ) * sc->get_tile_size().y );

      const float tex_left = static_cast<float>( tu * sc->get_tile_size().x );
      const float tex_top = static_cast<float>( tv * sc->get_tile_size().y );
      const float tex_right = static_cast<float>( ( tu + 1 ) * sc->get_tile_size().x );
      const float tex_bottom = static_cast<float>( ( tv + 1 ) * sc->get_tile_size().y );

      // define the 6 corners of the two triangles (counter-clockwise)
      m_vertices.append( { { left, top }, sf::Color::White, { tex_left, tex_top } } );
      m_vertices.append( { { right, top }, sf::Color::White, { tex_right, tex_top } } );
      m_vertices.append( { { left, bottom }, sf::Color::White, { tex_left, tex_bottom } } );
      m_vertices.append( { { left, bottom }, sf::Color::White, { tex_left, tex_bottom } } );
      m_vertices.append( { { right, top }, sf::Color::White, { tex_right, tex_top } } );
      m_vertices.append( { { right, bottom }, sf::Color::White, { tex_right, tex_bottom } } );
    }
  }

  SPDLOG_INFO( "Created tilemap: {} vertices", m_vertices.getVertexCount() );
}

} // namespace ProceduralMaze::Sprites::Containers