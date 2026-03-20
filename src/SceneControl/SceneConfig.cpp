#include <SceneControl/SceneConfig.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

using namespace ProceduralMaze::Scene;

namespace nlohmann
{
//! @brief ADL hook used via nlohmann::basic_json::get (see SceneConfig::load below)
template <>
struct adl_serializer<ProceduralMaze::Scene::SceneConfig::Config>
{
  static void from_json( const json &j, SceneConfig::Config &c )
  {

    if ( not j.contains( "scene_config" ) ) throw std::runtime_error( "Missing 'scene_config' from JSON scene config file" );
    const auto &config = j.at( "scene_config" );

    if ( not config.contains( "texture_path" ) ) throw std::runtime_error( "Missing 'texture_path' from JSON scene config file" );
    c.texture_path = config.at( "texture_path" ).get<std::string>();

    if ( not config.contains( "tile_size" ) ) throw std::runtime_error( "Missing 'tile_size' from JSON scene config file" );
    c.tile_size.x = config.at( "tile_size" ).at( "width" ).get<uint32_t>();
    c.tile_size.y = config.at( "tile_size" ).at( "height" ).get<uint32_t>();

    if ( not config.contains( "floor_tile_pool" ) ) throw std::runtime_error( "Missing 'floor_tile_pool' from JSON scene config file" );
    c.floor_tile_pool = config.at( "floor_tile_pool" ).get<std::vector<uint32_t>>();

    if ( not config.contains( "random_seed" ) ) throw std::runtime_error( "Missing 'random_seed' from JSON scene config file" );
    c.random_seed = config.value( "random_seed", 0u );

    if ( not config.contains( "map_size" ) ) throw std::runtime_error( "Missing 'map_size' from JSON scene config file" );
    c.map_size.x = config.at( "map_size" ).at( "width" ).get<uint32_t>();
    c.map_size.y = config.at( "map_size" ).at( "height" ).get<uint32_t>();

    if ( not config.contains( "player_start_position" ) ) throw std::runtime_error( "Missing 'player_start_position' from JSON scene config file" );
    c.player_start_position.x = config.at( "player_start_position" ).at( "x" ).get<uint32_t>();
    c.player_start_position.y = config.at( "player_start_position" ).at( "y" ).get<uint32_t>();

    // optional
    if ( config.contains( "exit_position" ) )
    {
      c.exit_position.x = config.at( "exit_position" ).at( "x" ).get<uint32_t>();
      c.exit_position.y = config.at( "exit_position" ).at( "y" ).get<uint32_t>();
    }

    // optional
    if ( config.contains( "multiblock_positions" ) )
    {
      for ( const auto &entry : config.at( "multiblock_positions" ) )
      {
        sf::Vector2u pos( entry.at( "position" ).at( "x" ).get<uint32_t>(), entry.at( "position" ).at( "y" ).get<uint32_t>() );
        c.multiblock_positions.insert( { entry.at( "name" ).get<std::string>(), pos } );
      }
    }

    // optional
    if ( config.contains( "npc_positions" ) )
    {
      for ( const auto &entry : config.at( "npc_positions" ) )
      {
        sf::Vector2u pos( entry.at( "position" ).at( "x" ).get<uint32_t>(), entry.at( "position" ).at( "y" ).get<uint32_t>() );
        c.npc_positions.insert( { entry.at( "name" ).get<std::string>(), pos } );
      }
    }
  }
};
} // namespace nlohmann

namespace ProceduralMaze::Scene
{
void SceneConfig::load( const std::filesystem::path &config_path )
{
  if ( not std::filesystem::exists( config_path ) )
  {
    SPDLOG_ERROR( "Config file does not exist: {}", config_path.string() );
    throw std::runtime_error( "Config file not found: " + config_path.string() );
  }

  std::ifstream file( config_path );
  if ( not file.is_open() )
  {
    SPDLOG_ERROR( "Unable to open config file: {}", config_path.string() );
    throw std::runtime_error( "Cannot open config file: " + config_path.string() );
  }

  //! @brief Attempt deserialise using the Argument-dependent lookup (ADL) serializer above
  try
  {
    nlohmann::json j;
    file >> j;
    m_config = j.get<SceneConfig::Config>();
  } catch ( const nlohmann::json::parse_error &e )
  {
    SPDLOG_ERROR( "JSON parse error in {}: {}", config_path.string(), e.what() );
    throw std::runtime_error( "Invalid JSON in config file" );
  }
}

std::pair<sf::Vector2u, sf::Vector2f> SceneConfig::get_player_start_position() const
{
  // clang-format off
  return { 
    m_config.player_start_position, 
    sf::Vector2f{ static_cast<float>( m_config.player_start_position.x * static_cast<float>(Constants::kGridSizePx.x) ),
                  static_cast<float>( m_config.player_start_position.y * static_cast<float>(Constants::kGridSizePx.y) ) } 
  };
  // clang-format on
}

std::pair<sf::Vector2u, sf::Vector2f> SceneConfig::get_map_size() const
{
  // clang-format off
  return { 
    m_config.map_size, 
    sf::Vector2f{ static_cast<float>( m_config.map_size.x * static_cast<float>(Constants::kGridSizePx.x) ),
                  static_cast<float>( m_config.map_size.y * static_cast<float>(Constants::kGridSizePx.y) ) } 
  };
  // clang-format on
}

std::pair<sf::Vector2u, sf::Vector2f> SceneConfig::get_exit_position() const
{
  // clang-format off
  return { 
    m_config.exit_position, 
    sf::Vector2f{ static_cast<float>( m_config.exit_position.x * static_cast<float>(Constants::kGridSizePx.x) ),
                  static_cast<float>( m_config.exit_position.y * static_cast<float>(Constants::kGridSizePx.y) ) } 
  };
  // clang-format on
}

std::vector<std::pair<sf::Vector2u, sf::Vector2f>> SceneConfig::get_sprite_position( std::string key )
{
  std::vector<std::pair<sf::Vector2u, sf::Vector2f>> results;
  auto [begin, end] = m_config.multiblock_positions.equal_range( key );
  for ( auto it = begin; it != end; ++it )
  {
    // clang-format off
    results.push_back({
        it->second, 
        sf::Vector2f {  static_cast<float>( it->second.x * static_cast<float>(Constants::kGridSizePx.x) ), 
                        static_cast<float>( it->second.y * static_cast<float>(Constants::kGridSizePx.y) ) }               
    });
    // clang-format on
  }
  return results;
}

std::vector<std::pair<sf::Vector2u, sf::Vector2f>> SceneConfig::get_npc_position( std::string key )
{
  std::vector<std::pair<sf::Vector2u, sf::Vector2f>> results;
  auto [begin, end] = m_config.npc_positions.equal_range( key );
  for ( auto it = begin; it != end; ++it )
  {
    // clang-format off
    results.push_back({
        it->second, 
        sf::Vector2f {  static_cast<float>( it->second.x * static_cast<float>(Constants::kGridSizePx.x) ), 
                        static_cast<float>( it->second.y * static_cast<float>(Constants::kGridSizePx.y) ) }               
    });
    // clang-format on
  }
  return results;
}

} // namespace ProceduralMaze::Scene