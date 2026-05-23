#include <Constants.hpp>
#include <SceneControl/SceneData.hpp>

#include <exception>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace nlohmann
{

//! @brief Deserialize Tilemaps
template <>
struct adl_serializer<Game::Scene::SceneData::Data>
{
  static void from_json( const json &j, Game::Scene::SceneData::Data &md )
  {

    //! @brief Misc layers
    auto get_list = [&]<typename T>( const json &j, const std::string &key, std::vector<T> &out )
    {
      if ( not j.contains( key ) ) throw std::runtime_error( "Missing JSON property '" + key + "'" );
      try
      {
        out = j.at( key ).get<std::vector<T>>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "JSON Error: '" + key + "': " + e.what() );
      }
    };

    //! @brief Solid layer
    auto get_solid_list = [&]( const json &j, std::vector<sf::FloatRect> &out )
    {
      if ( not j.contains( "objects" ) ) throw std::runtime_error( "Missing JSON property objects in solid object layer" );
      try
      {
        for ( auto &object : j.at( "objects" ) )
        {
          if ( not object.contains( "name" ) or object.at( "name" ).get<std::string>() != "solid" ) continue;
          if ( not object.contains( "x" ) ) throw std::runtime_error( "Missing JSON property 'x' in solid object layer" );
          if ( not object.contains( "y" ) ) throw std::runtime_error( "Missing JSON property 'y' in solid object layer" );
          if ( not object.contains( "width" ) ) throw std::runtime_error( "Missing JSON property 'width' in solid object layer" );
          if ( not object.contains( "height" ) ) throw std::runtime_error( "Missing JSON property 'height' in solid object layer" );
          sf::FloatRect r( { object.at( "x" ).get<float>(), object.at( "y" ).get<float>() },
                           { object.at( "width" ).get<float>(), object.at( "height" ).get<float>() } );
          out.push_back( r );
        }
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( std::string( "JSON Error: object layer - " ) + e.what() );
      }
    };

    //! @brief Multiblock layer
    auto get_multiblock_list = [&]( const json &j, std::multimap<Game::Sprites::SpriteMetaType, sf::Vector2f> &out )
    {
      if ( not j.contains( "objects" ) ) throw std::runtime_error( "Missing JSON property 'objects' in multiblock layer" );
      try
      {
        for ( auto &object : j.at( "objects" ) )
        {
          if ( not object.contains( "name" ) ) throw std::runtime_error( "Missing JSON property 'name' in multiblock object layer" );
          if ( not object.contains( "x" ) ) throw std::runtime_error( "Missing JSON property 'x' in multiblock object layer" );
          if ( not object.contains( "y" ) ) throw std::runtime_error( "Missing JSON property 'y' in multiblock object layer" );
          Game::Sprites::SpriteMetaType type( object.at( "name" ).get<std::string>() );
          sf::Vector2f pos( object.at( "x" ).get<float>(), object.at( "y" ).get<float>() );
          out.insert( { type, pos } );
        }
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( std::string( "JSON Error: object layer - " ) + e.what() );
      }
    };

    if ( not j.contains( "width" ) ) throw std::runtime_error( "Missing JSON property 'width' in scene tilemap layer" );
    if ( not j.contains( "height" ) ) throw std::runtime_error( "Missing JSON property 'height'  in scene tilemap layer" );
    md.map_size = sf::Vector2u( j.at( "width" ).get<uint32_t>(), j.at( "height" ).get<uint32_t>() );

    if ( j.contains( "layers" ) )
    {
      for ( const auto &layer : j.at( "layers" ) )
      {
        if ( not layer.contains( "name" ) ) throw std::runtime_error( "Missing JSON property 'name' in layer" );
        if ( layer.at( "name" ).get<std::string>() == "levelgen" ) { get_list( layer, "data", md.levelgen_tilelayer ); }
        if ( layer.at( "name" ).get<std::string>() == "player" ) { get_list( layer, "data", md.player_tilelayer ); }
        if ( layer.at( "name" ).get<std::string>() == "wall" ) { get_list( layer, "data", md.wall_tilelayer ); }
        if ( layer.at( "name" ).get<std::string>() == "solid" ) { get_solid_list( layer, md.solid_objectlayer ); }
        if ( layer.at( "name" ).get<std::string>() == "multiblock" ) { get_multiblock_list( layer, md.multiblock_objectlayer ); }
      }
      if ( md.levelgen_tilelayer.empty() ) { throw std::runtime_error( "Missing JSON property tilelayer: 'levelgen'" ); }
    }
  }
};
} // namespace nlohmann

namespace Game::Scene
{

SceneData::SceneData( std::filesystem::path map_file ) { deserialize( map_file ); }

void SceneData::deserialize( const std::filesystem::path &json_scene_file_path )
{
  SceneData::Data scene_tilemap;
  nlohmann::json scene_tilemap_json = load_json_file( json_scene_file_path );

  FloorTileSet floor_tileset;
  int wall_first_gid = 0;

  MainTileSet main_tileset;
  int main_ext_first_gid = 0;

  bool found_floor_tileset = false;
  bool found_wall_tileset = false;
  bool found_main_tileset = false;

  // deserialize the tilesets first
  if ( not scene_tilemap_json.contains( "tilesets" ) ) throw std::runtime_error( "Missing JSON property 'tilesets' in scene tilemap" );
  for ( const auto &tileset : scene_tilemap_json.at( "tilesets" ) )
  {
    // embedded tilesets
    if ( not found_floor_tileset ) { found_floor_tileset = deserialize_int_floor_tileset( tileset, floor_tileset ); }
    if ( not found_wall_tileset ) { found_wall_tileset = deserialize_int_wall_tileset( json_scene_file_path, tileset, wall_first_gid ); }

    // external 'main' tileset metadata
    if ( not found_main_tileset )
    {
      found_main_tileset = deserialize_ext_main_tileset( json_scene_file_path, tileset, main_tileset, main_ext_first_gid );
    }
  }
  if ( not found_floor_tileset ) { throw std::runtime_error( "JSON Error: missing floor tileset: " + json_scene_file_path.string() ); }
  if ( not found_main_tileset ) { throw std::runtime_error( "JSON Error: missing main tileset: " + json_scene_file_path.string() ); }
  if ( not found_wall_tileset ) { SPDLOG_WARN( "missing wall tileset: {}", json_scene_file_path.string() ); } // optional tileset

  // Now deserialize the overal tilemap file and populate it with the tilesets
  deserialize_tilemap( json_scene_file_path, scene_tilemap_json, scene_tilemap );
  scene_tilemap.wall_first_gid = wall_first_gid;
  scene_tilemap.floor_tileset = floor_tileset;
  scene_tilemap.main_first_gid = main_ext_first_gid;
  scene_tilemap.main_tileset = main_tileset;

  post_process_player_data( scene_tilemap );

  // assign it all to the class member
  m_map_data = scene_tilemap;
}

bool SceneData::deserialize_int_floor_tileset( const nlohmann::json &json, FloorTileSet &tileset )
{
  if ( json.contains( "name" ) and json.at( "name" ).get<std::string>() != "floor" ) return false;
  if ( not json.contains( "image" ) ) return false;

  std::string image = get_string( json, "image" );
  tileset.tileset_image = get_abs_path( image );

  tileset.tileset_pool.clear();
  tileset.tileset_pool = get_int_list_property( json, "pool" );
  if ( tileset.tileset_pool.empty() ) throw std::runtime_error( "Floor tileset 'pool' property is missing or empty" );
  tileset.tile_size = sf::Vector2u( get_int( json, "tilewidth" ), get_int( json, "tileheight" ) );

  return true;
}

bool SceneData::deserialize_int_wall_tileset( const std::filesystem::path &scene_tilemap_path, const nlohmann::json &tileset, int &wall_first_gid )
{
  bool result = false;
  if ( tileset.contains( "name" ) and tileset.at( "name" ).get<std::string>() == "wall" )
  {
    if ( not tileset.contains( "firstgid" ) ) throw std::runtime_error( "Missing JSON property 'firstgid' in " + scene_tilemap_path.string() );
    wall_first_gid = tileset.at( "firstgid" ).get<int>();
    result = true;
  }
  return result;
}

bool SceneData::deserialize_ext_main_tileset( const std::filesystem::path &scene_tilemap_path, const nlohmann::json &tileset,
                                              SceneData::MainTileSet &main_tileset, int &main_ext_first_gid )
{
  std::filesystem::path main_tileset_path;
  if ( not tileset.contains( "source" ) or not tileset.contains( "firstgid" ) ) return false;

  std::filesystem::path source = get_string( tileset, "source" );

  // We only expect one external tileset so just return early if others are found
  if ( source.filename() != "main.json" ) return false;

  main_tileset_path = Constants::res_dir / "scenes" / source;
  main_ext_first_gid = tileset.at( "firstgid" ).get<int>();

  if ( main_tileset_path.empty() ) { throw std::runtime_error( "Cannot find file 'main.json' from " + scene_tilemap_path.string() ); }

  // now deserialize the external main tileset json contents
  try
  {
    nlohmann::json main_tileset_json = load_json_file( main_tileset_path );

    if ( not main_tileset_json.contains( "tiles" ) ) throw std::runtime_error( "Missing JSON property 'tiles' in main tileset." );
    for ( const auto &tile : main_tileset_json.at( "tiles" ) )
    {
      auto id = get_int( tile, "id" );
      auto type = get_string( tile, "type" );

      if ( type == "void" ) { main_tileset.void_tile_id = id; }
      else if ( type == "wall" ) { main_tileset.wall_tile_id = id; }
      else if ( type == "open" ) { main_tileset.open_tile_id = id; }
      else if ( type == "spawn" ) { main_tileset.spawn_tile_id = id; }
      else if ( type == "player" ) { main_tileset.player_tile_id = id; }
      else if ( type == "exit" ) { main_tileset.exit_tile_id = id; }
      else if ( type == "reserved" ) { main_tileset.reserved_tile_id = id; }
      else { SPDLOG_WARN( "Unknown 'type' found in main tileset: {}", type ); }
    }
    // 'main_tileset.wall_tile_id' should default to zero so no check needed
    if ( main_tileset.wall_tile_id == 0 ) { throw std::runtime_error( "Missing JSON property 'type.wall' in main tileset" ); }
    if ( main_tileset.open_tile_id == 0 ) { throw std::runtime_error( "Missing JSON property 'type.open' in main tileset" ); }
    if ( main_tileset.spawn_tile_id == 0 ) { throw std::runtime_error( "Missing JSON property 'type.spawn' in main tileset" ); }
    if ( main_tileset.player_tile_id == 0 ) { throw std::runtime_error( "Missing JSON property 'type.player' in main tileset" ); }
    if ( main_tileset.exit_tile_id == 0 ) { throw std::runtime_error( "Missing JSON property 'type.exit' in main tileset" ); }
    if ( main_tileset.reserved_tile_id == 0 ) { throw std::runtime_error( "Missing JSON property 'type.reserved' in main tileset" ); }
    main_tileset.tile_size = sf::Vector2u( get_int( main_tileset_json, "tilewidth" ), get_int( main_tileset_json, "tileheight" ) );

  } catch ( std::exception &e )
  {
    throw std::runtime_error( "Error in main tileset '" + main_tileset_path.string() + "': " + e.what() );
  }

  return true;
}

void SceneData::deserialize_tilemap( const std::filesystem::path &scene_tilemap_path, const nlohmann::json &scene_tilemap_json,
                                     SceneData::Data &scene_tilemap )
{
  try
  {
    scene_tilemap = scene_tilemap_json.get<SceneData::Data>();
  } catch ( std::exception &e )
  {
    throw std::runtime_error( "Error in Tilemap '" + scene_tilemap_path.string() + "': " + e.what() );
  }
}

void SceneData::post_process_player_data( SceneData::Data &scene_tilemap )
{
  bool found_player_start_position = false;
  for ( auto [i, tile] : std::views::enumerate( scene_tilemap.player_tilelayer ) )
  {
    if ( scene_tilemap.main_tileset.player_tile_id == tile - scene_tilemap.main_first_gid )
    {
      int col = i % scene_tilemap.map_size.x; // wraps back to zero every 'w' tiles
      int row = i / scene_tilemap.map_size.x; // increments every 'w' tiles
      sf::Vector2u result( col, row );
      scene_tilemap.player_start_position = result;
      found_player_start_position = true;
      break; // stop at the first player position
    }
  }
  if ( not found_player_start_position ) { SPDLOG_WARN( "No player start tile found in 'player' layer, defaulting to (0,0)" ); }
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

} // namespace Game::Scene