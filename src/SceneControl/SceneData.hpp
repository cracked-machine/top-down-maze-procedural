#ifndef SRC_SCENECONTROL_SCENEDATA_HPP_
#define SRC_SCENECONTROL_SCENEDATA_HPP_

#include <SFML/Graphics/Rect.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <nlohmann/json_fwd.hpp>

namespace ProceduralMaze::Scene
{

class SceneData
{
public:
  struct MainTileSet
  {
    int void_tile_id{ 0 };
    int wall_tile_id{ 0 };
    int open_tile_id{ 0 };
    int spawn_tile_id{ 0 };
    int player_tile_id{ 0 };
    int exit_tile_id{ 0 };
    sf::Vector2u tile_size;
  };

  struct FloorTileSet
  {
    std::filesystem::path tileset_image;
    std::vector<int> tileset_pool;
    sf::Vector2u tile_size;
  };

  struct Data
  {
    int main_first_gid{ 0 };
    sf::Vector2u map_size;
    std::vector<int> levelgen_tilelayer;
    std::vector<int> player_tilelayer;

    int wall_first_gid{ 0 };
    std::vector<int> wall_tilelayer;

    std::vector<sf::FloatRect> solid_objectlayer;

    std::multimap<Sprites::SpriteMetaType, sf::Vector2f> multiblock_objectlayer;

    sf::Vector2u player_start_position{ 0, 0 };
    sf::Vector2u exit_position;
    MainTileSet main_tileset;
    FloorTileSet floor_tileset;
  };

  SceneData( std::filesystem::path map_file );

  int void_tile_id() const { return m_map_data.main_tileset.void_tile_id + m_map_data.main_first_gid; }
  int wall_tile_id() const { return m_map_data.main_tileset.wall_tile_id + m_map_data.main_first_gid; }
  int open_tile_id() const { return m_map_data.main_tileset.open_tile_id + m_map_data.main_first_gid; }
  int spawn_tile_id() const { return m_map_data.main_tileset.spawn_tile_id + m_map_data.main_first_gid; }
  int player_tile_id() const { return m_map_data.main_tileset.player_tile_id + m_map_data.main_first_gid; }
  int exit_tile_id() const { return m_map_data.main_tileset.exit_tile_id + m_map_data.main_first_gid; }

  std::multimap<Sprites::SpriteMetaType, sf::Vector2f> multiblock_objectlayer() const { return m_map_data.multiblock_objectlayer; }
  std::vector<sf::FloatRect> solid_objectlayer() const { return m_map_data.solid_objectlayer; }
  std::vector<int> levelgen_tilelayer() const { return m_map_data.levelgen_tilelayer; }
  std::vector<int> wall_tilelayer() const { return m_map_data.wall_tilelayer; }
  int wall_first_gid() const { return m_map_data.wall_first_gid; }
  std::filesystem::path floor_tileset_image() const { return m_map_data.floor_tileset.tileset_image; }
  std::vector<int> floor_tileset_pool() const { return m_map_data.floor_tileset.tileset_pool; }
  std::pair<sf::Vector2u, sf::Vector2f> get_player_start_position() const;

  //! @brief  Get the map size as grid position and pixel position (x16)
  //!         For example, {50,70} and {800.0,1120.0}
  //! @return std::pair<sf::Vector2u, sf::Vector2f>
  std::pair<sf::Vector2u, sf::Vector2f> map_size() const;

private:
  nlohmann::json load_json_file( const std::filesystem::path &json_file );

  //! @brief Get the tilesets and tilelayers from the Tiled json file
  //! @param scene_tiledata_path
  void deserialize( const std::filesystem::path &scene_tiledata_path );

  //! @brief Get the embedded floor tileset from the JSON object
  //! @param scene_tilemap_path Used for meaningful logging
  //! @param tileset The JSON input
  //! @param floor_tileset The output
  //! @return true
  //! @return false
  bool deserialize_int_floor_tileset( const std::filesystem::path &scene_tilemap_path, const nlohmann::json &tileset, FloorTileSet &floor_tileset );

  //! @brief Get the embedded wall tileset from the JSON object
  //! @param tileset The JSON input
  //! @param wall_first_gid The output
  //! @return true
  //! @return false
  bool deserialize_int_wall_tileset( const std::filesystem::path &scene_tilemap_path, const nlohmann::json &tileset, int &wall_first_gid );

  //! @brief Get the external main tileset from the JSON object
  //! @param scene_tilemap_path Used for meaningful logging
  //! @param tileset The JSON input
  //! @param main_tileset The output
  //! @param main_ext_first_gid The output
  //! @return true
  //! @return false
  bool deserialize_ext_main_tileset( const std::filesystem::path &scene_tilemap_path, const nlohmann::json &tileset,
                                     SceneData::MainTileSet &main_tileset, int &main_ext_first_gid );

  //! @brief Deserialize the scene tilemap json and copy in 'floor_tileset' and 'main_tileset'
  //! @param scene_tilemap_path Used for meaningful logging
  //! @param scene_tilemap_json The JSON input
  //! @param scene_tilemap The output
  void deserialize_tilemap( const std::filesystem::path &scene_tilemap_path, const nlohmann::json &scene_tilemap_json,
                            SceneData::Data &scene_tilemap );

  //! @brief Post-process scene_tilemap to get the player start position from player_tilelayer
  //! @param scene_tilemap
  void post_process_player_data( SceneData::Data &scene_tilemap );

  Data m_map_data;
  std::filesystem::path m_main_tileset_path{ "res/scenes/Tilesets/main.json" };
};

using SceneMapSharedPtr = std::shared_ptr<SceneData>;
using SceneMapWeakPtr = std::weak_ptr<SceneData>;

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SCENEDATA_HPP_