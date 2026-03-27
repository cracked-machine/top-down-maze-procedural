#include <SceneControl/SceneMap.hpp>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace nlohmann
{
//! @brief ADL hook used via nlohmann::basic_json::get (see SceneConfig::load below)
template <>
struct adl_serializer<ProceduralMaze::Scene::SceneMap::MapData>
{
  static void from_json( const json &j, ProceduralMaze::Scene::SceneMap::MapData &c )
  {
    //! @brief helper for error checking on JSON Single field types
    auto get_field = [&]<typename T>( const json &j, const std::string &key, T &out )
    {
      if ( !j.contains( key ) ) throw std::runtime_error( "Missing '" + key + "' from JSON scene config file" );
      try
      {
        out = j.at( key ).get<T>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "Error parsing '" + key + "': " + e.what() );
      }
    };

    //! @brief lambda helper for error checking on JSON list
    auto get_int_list = [&]<typename T>( const json &j, const std::string &key, std::vector<T> &out )
    {
      if ( not j.contains( key ) ) return;
      try
      {
        out = j.at( key ).get<std::vector<T>>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "Error parsing JSON sprites file: '" + key + "': " + e.what() );
      }
    };

    // deserialize
    get_field( j, "width", c.width );
    get_field( j, "height", c.height );
    if ( j.contains( "layers" ) )
    {
      for ( const auto &layer : j.at( "layers" ) )
      {
        get_int_list( layer, "data", c.map );
        if ( !c.map.empty() ) break; // take the first layer with data
      }
    }
    if ( j.contains( "tilesets" ) )
    {
      for ( const auto &tileset : j.at( "tilesets" ) )
      {
        if ( not tileset.contains( "firstgid" ) ) throw std::runtime_error( "Error parsing JSON file - Missing tileset property: 'firstgid'" );
        ;
        auto firstgid = tileset.at( "firstgid" ).get<int>();
        for ( const auto &tile : tileset.at( "tiles" ) )
        {
          if ( not tile.contains( "type" ) ) throw std::runtime_error( "Error parsing JSON file - Missing tile property: 'id'" );
          auto id = tile.at( "id" ).get<int>();
          auto type = tile.at( "type" ).get<std::string>();
          if ( type == "void" ) { c.voididx = id + firstgid; }
          if ( type == "wall" ) { c.wallidx = id + firstgid; }
          if ( type == "open" ) { c.openidx = id + firstgid; }
          if ( type == "spawn" ) { c.spawnidx = id + firstgid; }
        }
      }
    }
  }
};
} // namespace nlohmann

namespace ProceduralMaze::Scene
{

SceneMap::SceneMap( std::filesystem::path map_file ) { load_config( map_file ); }

void SceneMap::load_config( const std::filesystem::path &config_path )
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
    m_map_data = j.get<SceneMap::MapData>();
  } catch ( const ::nlohmann::json::parse_error &e )
  {
    SPDLOG_ERROR( "JSON parse error in {}: {}", config_path.string(), e.what() );
    throw std::runtime_error( "Invalid JSON in config file" );
  }
}

} // namespace ProceduralMaze::Scene