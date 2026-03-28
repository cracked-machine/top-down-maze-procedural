#include <Constants.hpp>
#include <SceneControl/SceneData.hpp>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace nlohmann
{
//! @brief ADL hook used via nlohmann::basic_json::get (see SceneMap::load below)
template <>
struct adl_serializer<ProceduralMaze::Scene::SceneData::MainTileSet>
{
  static void from_json( const json &j, ProceduralMaze::Scene::SceneData::MainTileSet &ts )
  {
    if ( not j.contains( "tiles" ) ) throw std::runtime_error( "JSON Error - Missing field 'tiles' in main tileset." );
    for ( const auto &tile : j.at( "tiles" ) )
    {
      auto id = tile.at( "id" ).get<int>();
      auto type = tile.at( "type" ).get<std::string>();

      if ( type == "void" ) { ts.void_tile_id = id; }
      else if ( type == "wall" ) { ts.wall_tile_id = id; }
      else if ( type == "open" ) { ts.open_tile_id = id; }
      else if ( type == "spawn" ) { ts.spawn_tile_id = id; }
      else if ( type == "player" ) { ts.player_tile_id = id; }
      else if ( type == "exit" ) { ts.exit_tile_id = id; }
    }
    if ( not j.contains( "tilewidth" ) ) throw std::runtime_error( "JSON Error - Missing field 'tilewidth' in main tileset." );
    if ( not j.contains( "tileheight" ) ) throw std::runtime_error( "JSON Error - Missing field 'tileheight' in main tileset." );
    ts.tile_size = sf::Vector2u( j.at( "tilewidth" ).get<uint32_t>(), j.at( "tileheight" ).get<uint32_t>() );
  }
};

//! @brief ADL hook used via nlohmann::basic_json::get (see SceneMap::load below)
template <>
struct adl_serializer<ProceduralMaze::Scene::SceneData::FloorTileSet>
{
  static void from_json( const json &j, ProceduralMaze::Scene::SceneData::FloorTileSet &ts )
  {
    if ( not j.contains( "image" ) ) throw std::runtime_error( "JSON Error - Missing tileset property: 'tilesets.image'" );
    std::string image = j.at( "image" ).get<std::string>();

    // strip the relative dots and prepend with the resource dir
    std::filesystem::path result;
    for ( const auto &part : std::filesystem::path( image ) )
    {
      if ( part == ".." || part == "." ) continue;
      result /= part;
    }
    ts.tileset_image = ProceduralMaze::Constants::res_dir / result;
    for ( const auto &property : j.at( "properties" ) )
    {
      if ( not property.contains( "name" ) ) { throw std::runtime_error( "JSON Error - Missing tileset property: 'tilesets.properties.name'" ); }
      if ( not property.contains( "value" ) ) { throw std::runtime_error( "JSON Error - Missing tileset property: 'tilesets.properties.value'" ); }
      if ( property.at( "name" ).get<std::string>() != "pool" ) continue;

      // confusingly the 'value' list contains 'value' objects
      ts.tileset_pool.clear();
      for ( const auto &value_object : property.at( "value" ) )
      {
        ts.tileset_pool.push_back( value_object.at( "value" ).get<int>() );
      }
    }
    if ( not j.contains( "tilewidth" ) ) throw std::runtime_error( "JSON Error - Missing field 'tilewidth' in floor tileset." );
    if ( not j.contains( "tileheight" ) ) throw std::runtime_error( "JSON Error - Missing field 'tileheight' in floor tileset." );
    ts.tile_size = sf::Vector2u( j.at( "tilewidth" ).get<uint32_t>(), j.at( "tileheight" ).get<uint32_t>() );
  }
};

//! @brief ADL hook used via nlohmann::basic_json::get (see SceneMap::load below)
template <>
struct adl_serializer<ProceduralMaze::Scene::SceneData::Data>
{
  static void from_json( const json &j, ProceduralMaze::Scene::SceneData::Data &md )
  {

    //! @brief lambda helper for error checking on JSON list
    auto get_int_list = [&]<typename T>( const json &j, const std::string &key, std::vector<T> &out )
    {
      if ( not j.contains( key ) ) throw std::runtime_error( "JSON Error - Missing field '" + key + "'" );
      try
      {
        out = j.at( key ).get<std::vector<T>>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "JSON Error: '" + key + "': " + e.what() );
      }
    };

    //! @brief lambda helper for error checking on JSON list
    auto get_solid_list = [&]( const json &j, std::vector<sf::FloatRect> &out )
    {
      if ( not j.contains( "objects" ) ) throw std::runtime_error( "JSON Error - Missing objects in solid object layer" );
      try
      {
        for ( auto &object : j.at( "objects" ) )
        {
          if ( not object.contains( "name" ) or object.at( "name" ).get<std::string>() != "solid" ) continue;
          if ( not object.contains( "x" ) ) throw std::runtime_error( "JSON Error - Missing 'x' field in solid object layer" );
          if ( not object.contains( "y" ) ) throw std::runtime_error( "JSON Error - Missing 'y' field in solid object layer" );
          if ( not object.contains( "width" ) ) throw std::runtime_error( "JSON Error - Missing 'width' field in solid object layer" );
          if ( not object.contains( "height" ) ) throw std::runtime_error( "JSON Error - Missing 'height' field in solid object layer" );
          sf::FloatRect r( { object.at( "x" ).get<float>(), object.at( "y" ).get<float>() },
                           { object.at( "width" ).get<float>(), object.at( "height" ).get<float>() } );
          out.push_back( r );
        }
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( std::string( "JSON Error: object layer - " ) + e.what() );
      }
    };

    //! @brief lambda helper for error checking on JSON list
    auto get_multiblock_list = [&]( const json &j, std::multimap<ProceduralMaze::Sprites::SpriteMetaType, sf::Vector2f> &out )
    {
      if ( not j.contains( "objects" ) ) throw std::runtime_error( "JSON Error - Missing objects in multiblock object layer" );
      try
      {
        for ( auto &object : j.at( "objects" ) )
        {
          if ( not object.contains( "name" ) ) throw std::runtime_error( "JSON Error - Missing 'name' field in multiblock object layer" );
          if ( not object.contains( "x" ) ) throw std::runtime_error( "JSON Error - Missing 'x' field in multiblock object layer" );
          if ( not object.contains( "y" ) ) throw std::runtime_error( "JSON Error - Missing 'y' field in multiblock object layer" );
          ProceduralMaze::Sprites::SpriteMetaType type( object.at( "name" ).get<std::string>() );
          sf::Vector2f pos( object.at( "x" ).get<float>(), object.at( "y" ).get<float>() );
          out.insert( { type, pos } );
        }
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( std::string( "JSON Error: object layer - " ) + e.what() );
      }
    };

    // deserialize
    if ( not j.contains( "width" ) ) throw std::runtime_error( "Missing 'width' from JSON scene config file" );
    if ( not j.contains( "height" ) ) throw std::runtime_error( "Missing 'height' from JSON scene config file" );
    md.map_size = sf::Vector2u( j.at( "width" ).get<uint32_t>(), j.at( "height" ).get<uint32_t>() );

    if ( j.contains( "layers" ) )
    {
      for ( const auto &layer : j.at( "layers" ) )
      {
        if ( layer.at( "name" ).get<std::string>() == "levelgen" ) { get_int_list( layer, "data", md.levelgen_tilelayer ); }
        if ( layer.at( "name" ).get<std::string>() == "player" ) { get_int_list( layer, "data", md.player_tilelayer ); }
        if ( layer.at( "name" ).get<std::string>() == "wall" ) { get_int_list( layer, "data", md.wall_tilelayer ); }
        if ( layer.at( "name" ).get<std::string>() == "solid" ) { get_solid_list( layer, md.solid_objectlayer ); }
        if ( layer.at( "name" ).get<std::string>() == "multiblock" ) { get_multiblock_list( layer, md.multiblock_objectlayer ); }
      }
      if ( md.levelgen_tilelayer.empty() ) { throw std::runtime_error( "JSON Error - Missing tilelayer: 'levelgen'" ); }
    }
  }
};
} // namespace nlohmann

namespace ProceduralMaze::Scene
{

SceneData::SceneData( std::filesystem::path map_file ) { load_config( map_file ); }

std::ifstream SceneData::load_json_file( std::filesystem::path json_file )
{
  if ( not std::filesystem::exists( json_file ) )
  {
    SPDLOG_ERROR( "Config file does not exist: {}", json_file.string() );
    throw std::runtime_error( "Config file not found: " + json_file.string() );
  }

  std::ifstream main_tileset_filestream( json_file );
  if ( not main_tileset_filestream.is_open() )
  {
    SPDLOG_ERROR( "Unable to open config file: {}", json_file.string() );
    throw std::runtime_error( "Cannot open config file: " + json_file.string() );
  }
  return main_tileset_filestream;
}

void SceneData::load_config( const std::filesystem::path &scene_tilemap_path )
{
  std::ifstream main_tileset_fs;
  auto scene_tilemap_fs = load_json_file( scene_tilemap_path );

  //! @brief Attempt deserialise using the Argument-dependent lookup (ADL) serializer above
  try
  {
    FloorTileSet floor_tileset;
    nlohmann::json scene_tilemap_json;
    scene_tilemap_fs >> scene_tilemap_json;
    int first_gid = 0;
    int wall_first_gid = 0;

    for ( const auto &tileset : scene_tilemap_json.at( "tilesets" ) )
    {

      // deserialize the embedded tilesets
      if ( tileset.contains( "name" ) and tileset.at( "name" ).get<std::string>() == "floor" )
      {
        floor_tileset = tileset.get<SceneData::FloorTileSet>();
      }
      if ( tileset.contains( "name" ) and tileset.at( "name" ).get<std::string>() == "wall" )
      {
        wall_first_gid = tileset.at( "firstgid" ).get<int>();
      }

      // deserialize the external main tileset file
      if ( tileset.contains( "source" ) and tileset.contains( "firstgid" ) )
      {
        std::filesystem::path source = tileset.at( "source" ).get<std::string>();
        if ( source.filename() == "main.json" )
        {
          std::filesystem::path main_tileset_path = "res/scenes" / source;
          main_tileset_fs = load_json_file( main_tileset_path );
          first_gid = tileset.at( "firstgid" ).get<int>();
        }
      }
    }

    if ( not main_tileset_fs.is_open() ) { throw std::runtime_error( "JSON Error - No external main tileset ('source') found in tilesets" ); }

    // now deserialize the external main tileset json contents
    nlohmann::json main_tileset_json;
    main_tileset_fs >> main_tileset_json;
    auto main_tileset = main_tileset_json.get<SceneData::MainTileSet>();

    // deserialize the scene tilemap json and copy in 'floor_tileset' and 'main_tileset'
    auto scene_tilemap = scene_tilemap_json.get<SceneData::Data>();
    SPDLOG_INFO( "Solid list {}", scene_tilemap.solid_objectlayer.size() );
    scene_tilemap.wall_first_gid = wall_first_gid;
    scene_tilemap.first_gid = first_gid;
    scene_tilemap.floor_tileset = floor_tileset;
    scene_tilemap.main_tileset = main_tileset;

    // post-process the player tilelayer data to get the player start position
    for ( auto [i, tile] : std::views::enumerate( scene_tilemap.player_tilelayer ) )
    {
      if ( scene_tilemap.main_tileset.player_tile_id == tile - scene_tilemap.first_gid )
      {
        int col = i % scene_tilemap.map_size.x; // wraps back to zero every 'w' tiles
        int row = i / scene_tilemap.map_size.x; // increments every 'w' tiles
        sf::Vector2u result( col, row );
        scene_tilemap.player_start_position = result;
        break; // stop at the first player position
      }
    }
    if ( scene_tilemap.player_start_position == sf::Vector2u( 0, 0 ) )
    {
      SPDLOG_WARN( "No player start tile found in 'player' layer, defaulting to (0,0)" );
    }

    // assign it all to the class member
    m_map_data = scene_tilemap;

  } catch ( const ::nlohmann::json::parse_error &e )
  {
    SPDLOG_ERROR( "JSON parse error in {}: {}", scene_tilemap_path.string(), e.what() );
    throw std::runtime_error( "Invalid JSON in config file" );
  }
}

std::pair<sf::Vector2u, sf::Vector2f> SceneData::get_player_start_position() const
{
  // clang-format off
  return { 
    m_map_data.player_start_position, 
    sf::Vector2f{ static_cast<float>( m_map_data.player_start_position.x * static_cast<float>(Constants::kGridSizePx.x) ),
                  static_cast<float>( m_map_data.player_start_position.y * static_cast<float>(Constants::kGridSizePx.y) ) } 
  };
  // clang-format on
}

std::pair<sf::Vector2u, sf::Vector2f> SceneData::map_size() const
{
  // clang-format off
  return { 
    m_map_data.map_size, 
    sf::Vector2f{ static_cast<float>( m_map_data.map_size.x * static_cast<float>(Constants::kGridSizePx.x) ),
                  static_cast<float>( m_map_data.map_size.y * static_cast<float>(Constants::kGridSizePx.y) ) } 
  };
  // clang-format on
}

} // namespace ProceduralMaze::Scene