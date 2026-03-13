#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Constants.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <ReservedPosition.hpp>
#include <Sprites/TileMap.hpp>

#include <SFML/Graphics/Color.hpp>
#include <entt/entity/registry.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <stdexcept>

namespace ProceduralMaze::Sprites::Containers
{

void TileMap::load( const PathFinding::SpatialHashGrid &void_sm, sf::Vector2u map_dimensions, const std::filesystem::path &config_path )
{
  try
  {
    m_config = load_config( config_path );
    m_config.map_dimensions = map_dimensions;
    // Validate texture file exists
    if ( !std::filesystem::exists( m_config.texture_path ) )
    {
      SPDLOG_CRITICAL( "Texture file does not exist: {}", m_config.texture_path.string() );
      throw std::runtime_error( "Texture file not found" );
    }

    // load the tileset texture first
    if ( !m_tileset.loadFromFile( m_config.texture_path ) )
    {
      SPDLOG_CRITICAL( "Unable to load tileset texture file: {}", m_config.texture_path.string() );
      throw std::runtime_error( "Failed to load texture" );
    }

    create( void_sm );
  } catch ( const std::exception &e )
  {
    SPDLOG_CRITICAL( "Failed to initialize TileMap: {}", e.what() );
    throw;
  }
}

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

TileMap::TileMapConfig TileMap::load_config( const std::filesystem::path &config_path )
{
  if ( !std::filesystem::exists( config_path ) )
  {
    SPDLOG_ERROR( "Config file does not exist: {}", config_path.string() );
    throw std::runtime_error( "Config file not found: " + config_path.string() );
  }

  std::ifstream file( config_path );
  if ( !file.is_open() )
  {
    SPDLOG_ERROR( "Unable to open config file: {}", config_path.string() );
    throw std::runtime_error( "Cannot open config file: " + config_path.string() );
  }

  nlohmann::json j;
  try
  {
    file >> j;
  } catch ( const nlohmann::json::parse_error &e )
  {
    SPDLOG_ERROR( "JSON parse error in {}: {}", config_path.string(), e.what() );
    throw std::runtime_error( "Invalid JSON in config file" );
  }

  // Validate required fields exist
  if ( !j.contains( "tilemap" ) ) { throw std::runtime_error( "Missing 'tilemap' section in config" ); }

  const auto &tilemap = j["tilemap"];
  std::vector<std::string> required_fields = { "texture_path", "tile_size", "floor_tile_pool" };

  for ( const auto &field : required_fields )
  {
    if ( !tilemap.contains( field ) ) { throw std::runtime_error( "Missing required field: " + field ); }
  }

  TileMapConfig config;

  try
  {
    config.texture_path = std::filesystem::path( tilemap["texture_path"] );
    config.tile_size.x = tilemap["tile_size"]["width"];
    config.tile_size.y = tilemap["tile_size"]["height"];
    config.floor_tile_pool = tilemap["floor_tile_pool"].get<std::vector<unsigned int>>();
    config.random_seed = tilemap.value( "random_seed", 0 ); // Default to 0 if missing
  } catch ( const nlohmann::json::exception &e )
  {
    SPDLOG_ERROR( "Error parsing config values: {}", e.what() );
    throw std::runtime_error( "Invalid config values" );
  }

  // Validate config values
  if ( config.tile_size.x == 0 || config.tile_size.y == 0 ) { throw std::runtime_error( "Invalid tile size" ); }

  if ( config.floor_tile_pool.empty() ) { throw std::runtime_error( "Empty floor tile pool" ); }

  SPDLOG_DEBUG( "Loaded config: texture={}, tile_size={}x{}, pool_size={}", config.texture_path.string(), config.tile_size.x, config.tile_size.y,
                config.floor_tile_pool.size() );

  return config;
}

void TileMap::create( const PathFinding::SpatialHashGrid &void_sm )
{
  // Validate texture is loaded before creating vertices
  if ( m_tileset.getSize().x == 0 || m_tileset.getSize().y == 0 )
  {
    SPDLOG_ERROR( "Tileset texture not loaded or invalid" );
    return;
  }

  const sf::Vector2u texture_size = m_tileset.getSize();
  const unsigned int tiles_per_row = texture_size.x / m_config.tile_size.x;

  Cmp::RandomInt floortile_picker{ 0, static_cast<int>( m_config.floor_tile_pool.size() - 1 ) };
  // let json fix seed if specified as non-zero
  if ( m_config.random_seed != 0 )
  {
    floortile_picker.seed( static_cast<unsigned long>( m_config.random_seed ) );
    SPDLOG_DEBUG( "Using random seed: {}", m_config.random_seed );
  }

  // resize the vertex array for squares made of two triangles
  m_vertices.setPrimitiveType( sf::PrimitiveType::Triangles );

  SPDLOG_INFO( "Generating tilemap for {}x{}", m_config.map_dimensions.x, m_config.map_dimensions.y );
  for ( unsigned int w = 0; w < m_config.map_dimensions.x; w++ )
  {
    for ( unsigned int h = 0; h < m_config.map_dimensions.y; h++ )
    {
      const unsigned int tile_number = m_config.floor_tile_pool[floortile_picker.gen()];

      // check if we have anything in the void spatial map for this position
      Cmp::Position lookup_position( { w * Constants::kGridSizePxF.x, h * Constants::kGridSizePxF.y }, Constants::kGridSizePxF );

      auto results = void_sm.at( lookup_position );
      if ( not results.empty() ) continue;
      SPDLOG_INFO( "Lookup void {} in SM: {},{}", results.size(), lookup_position.x(), lookup_position.y() );

      // find its position in the tileset texture
      const unsigned int tu = tile_number % tiles_per_row;
      const unsigned int tv = tile_number / tiles_per_row;

      // Cache position calculations
      const float left = static_cast<float>( w * m_config.tile_size.x );
      const float top = static_cast<float>( h * m_config.tile_size.y );
      const float right = static_cast<float>( ( w + 1 ) * m_config.tile_size.x );
      const float bottom = static_cast<float>( ( h + 1 ) * m_config.tile_size.y );

      const float tex_left = static_cast<float>( tu * m_config.tile_size.x );
      const float tex_top = static_cast<float>( tv * m_config.tile_size.y );
      const float tex_right = static_cast<float>( ( tu + 1 ) * m_config.tile_size.x );
      const float tex_bottom = static_cast<float>( ( tv + 1 ) * m_config.tile_size.y );

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