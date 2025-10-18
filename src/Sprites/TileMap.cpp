#include <TileMap.hpp>
#include <fstream>
#include <stdexcept>

namespace ProceduralMaze::Sprites::Containers {

TileMap::TileMap( const std::filesystem::path &config_path )
{
  try
  {
    TileMapConfig config = load_config( config_path );
    initialize( config );
  }
  catch ( const std::exception &e )
  {
    SPDLOG_CRITICAL( "Failed to initialize TileMap: {}", e.what() );
    throw;
  }
}

void TileMap::create( sf::Vector2u tile_size, unsigned int width, unsigned int height )
{

  if ( width == 0 || height == 0 )
  {
    SPDLOG_ERROR( "Invalid dimensions: {}x{}", width, height );
    return;
  }

  if ( tile_size.x == 0 || tile_size.y == 0 )
  {
    SPDLOG_ERROR( "Invalid tile size: {}x{}", tile_size.x, tile_size.y );
    return;
  }

  // Validate texture is loaded before creating vertices
  if ( m_tileset.getSize().x == 0 || m_tileset.getSize().y == 0 )
  {
    SPDLOG_ERROR( "Tileset texture not loaded or invalid" );
    return;
  }

  const sf::Vector2u texture_size = m_tileset.getSize();
  const unsigned int tiles_per_row = texture_size.x / tile_size.x;
  const unsigned int tiles_per_col = texture_size.y / tile_size.y;
  const unsigned int max_tile_index = tiles_per_row * tiles_per_col - 1;

  // resize the vertex array to fit the level size
  m_vertices.setPrimitiveType( sf::PrimitiveType::Triangles );
  m_vertices.resize( width * height * 6 );

  // populate the vertex array, with two triangles per tile
  for ( unsigned int i = 0; i < width; ++i )
  {
    for ( unsigned int j = 0; j < height; ++j )
    {
      // get the current tile number
      const unsigned int tile_number = m_floortile_choices[i + j * width];

      // Validate tile index
      if ( tile_number > max_tile_index )
      {
        SPDLOG_WARN( "Tile index {} exceeds max index {} for texture size {}x{}", tile_number, max_tile_index,
                     texture_size.x, texture_size.y );
        continue; // Skip invalid tiles
      }

      // find its position in the tileset texture
      const unsigned int tu = tile_number % tiles_per_row;
      const unsigned int tv = tile_number / tiles_per_row;

      // get a pointer to the triangles' vertices of the current tile
      sf::Vertex *triangles = &m_vertices[( i + j * width ) * 6];

      // Cache position calculations
      const float left = static_cast<float>( i * tile_size.x );
      const float top = static_cast<float>( j * tile_size.y );
      const float right = static_cast<float>( ( i + 1 ) * tile_size.x );
      const float bottom = static_cast<float>( ( j + 1 ) * tile_size.y );

      const float tex_left = static_cast<float>( tu * tile_size.x );
      const float tex_top = static_cast<float>( tv * tile_size.y );
      const float tex_right = static_cast<float>( ( tu + 1 ) * tile_size.x );
      const float tex_bottom = static_cast<float>( ( tv + 1 ) * tile_size.y );

      // define the 6 corners of the two triangles (counter-clockwise)
      triangles[0].position = sf::Vector2f( left, top );
      triangles[1].position = sf::Vector2f( right, top );
      triangles[2].position = sf::Vector2f( left, bottom );
      triangles[3].position = sf::Vector2f( left, bottom );
      triangles[4].position = sf::Vector2f( right, top );
      triangles[5].position = sf::Vector2f( right, bottom );

      // define the 6 matching texture coordinates
      triangles[0].texCoords = sf::Vector2f( tex_left, tex_top );
      triangles[1].texCoords = sf::Vector2f( tex_right, tex_top );
      triangles[2].texCoords = sf::Vector2f( tex_left, tex_bottom );
      triangles[3].texCoords = sf::Vector2f( tex_left, tex_bottom );
      triangles[4].texCoords = sf::Vector2f( tex_right, tex_top );
      triangles[5].texCoords = sf::Vector2f( tex_right, tex_bottom );
    }
  }

  SPDLOG_INFO( "Created tilemap: {}x{} tiles, {} vertices", width, height, m_vertices.getVertexCount() );
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
  }
  catch ( const nlohmann::json::parse_error &e )
  {
    SPDLOG_ERROR( "JSON parse error in {}: {}", config_path.string(), e.what() );
    throw std::runtime_error( "Invalid JSON in config file" );
  }

  // Validate required fields exist
  if ( !j.contains( "tilemap" ) ) { throw std::runtime_error( "Missing 'tilemap' section in config" ); }

  const auto &tilemap = j["tilemap"];
  std::vector<std::string> required_fields = { "texture_path", "tile_size", "map_dimensions", "floor_tile_pool" };

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
    config.map_dimensions.x = tilemap["map_dimensions"]["width"];
    config.map_dimensions.y = tilemap["map_dimensions"]["height"];
    config.floor_tile_pool = tilemap["floor_tile_pool"].get<std::vector<unsigned int>>();
    config.random_seed = tilemap.value( "random_seed", 0 ); // Default to 0 if missing
  }
  catch ( const nlohmann::json::exception &e )
  {
    SPDLOG_ERROR( "Error parsing config values: {}", e.what() );
    throw std::runtime_error( "Invalid config values" );
  }

  // Validate config values
  if ( config.tile_size.x == 0 || config.tile_size.y == 0 ) { throw std::runtime_error( "Invalid tile size" ); }

  if ( config.map_dimensions.x == 0 || config.map_dimensions.y == 0 )
  {
    throw std::runtime_error( "Invalid map dimensions" );
  }

  if ( config.floor_tile_pool.empty() ) { throw std::runtime_error( "Empty floor tile pool" ); }

  SPDLOG_INFO( "Loaded config: texture={}, tile_size={}x{}, map={}x{}, pool_size={}", config.texture_path.string(),
               config.tile_size.x, config.tile_size.y, config.map_dimensions.x, config.map_dimensions.y,
               config.floor_tile_pool.size() );

  return config;
}

void TileMap::initialize( const TileMapConfig &config )
{
  // Validate texture file exists
  if ( !std::filesystem::exists( config.texture_path ) )
  {
    SPDLOG_CRITICAL( "Texture file does not exist: {}", config.texture_path.string() );
    throw std::runtime_error( "Texture file not found" );
  }

  // load the tileset texture first
  if ( !m_tileset.loadFromFile( config.texture_path ) )
  {
    SPDLOG_CRITICAL( "Unable to load tileset texture file: {}", config.texture_path.string() );
    throw std::runtime_error( "Failed to load texture" );
  }

  // Reserve capacity for performance
  const size_t total_tiles = static_cast<size_t>( config.map_dimensions.x ) * config.map_dimensions.y;
  m_floortile_choices.reserve( total_tiles );

  Cmp::Random floortile_picker{ 0, static_cast<int>( config.floor_tile_pool.size() - 1 ) };

  // let json fix seed if specified as non-zero
  if ( config.random_seed != 0 )
  {
    floortile_picker.seed( static_cast<unsigned long>( config.random_seed ) );
    SPDLOG_DEBUG( "Using random seed: {}", config.random_seed );
  }

  // pick random choices from the floor tile pool of indices
  for ( unsigned int x = 0; x < config.map_dimensions.x; x++ )
  {
    for ( unsigned int y = 0; y < config.map_dimensions.y; y++ )
    {
      m_floortile_choices.push_back( config.floor_tile_pool[floortile_picker.gen()] );
    }
  }

  if ( m_floortile_choices.size() != total_tiles )
  {
    SPDLOG_CRITICAL( "Tile choice pool size {} does not match expected total tiles {}", m_floortile_choices.size(),
                     total_tiles );
    throw std::runtime_error( "Tile choice pool size mismatch" );
  }

  create( config.tile_size, config.map_dimensions.x, config.map_dimensions.y );
}

} // namespace ProceduralMaze::Sprites::Containers