#ifndef SRC_SCENECONTROL_SCENEDATA_HPP_
#define SRC_SCENECONTROL_SCENEDATA_HPP_

#include <SFML/Graphics/Rect.hpp>
#include <Sprites/SpriteMetaType.hpp>
namespace ProceduralMaze::Scene
{

class SceneData
{
public:
  struct MainTileSet
  {
    int void_tile_id;
    int wall_tile_id;
    int open_tile_id;
    int spawn_tile_id;
    int player_tile_id;
    int exit_tile_id;
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
    int first_gid;
    sf::Vector2u map_size;
    std::vector<int> levelgen_tilelayer;
    std::vector<int> player_tilelayer;

    int wall_first_gid;
    std::vector<int> wall_tilelayer;

    std::vector<sf::FloatRect> solid_objectlayer;

    std::multimap<Sprites::SpriteMetaType, sf::Vector2f> multiblock_objectlayer;

    sf::Vector2u player_start_position;
    sf::Vector2u exit_position;
    MainTileSet main_tileset;
    FloorTileSet floor_tileset;
  };

  SceneData( std::filesystem::path map_file );

  int void_tile_id() const { return m_map_data.main_tileset.void_tile_id + m_map_data.first_gid; }
  int wall_tile_id() const { return m_map_data.main_tileset.wall_tile_id + m_map_data.first_gid; }
  int open_tile_id() const { return m_map_data.main_tileset.open_tile_id + m_map_data.first_gid; }
  int spawn_tile_id() const { return m_map_data.main_tileset.spawn_tile_id + m_map_data.first_gid; }
  int player_tile_id() const { return m_map_data.main_tileset.player_tile_id + m_map_data.first_gid; }
  int exit_tile_id() const { return m_map_data.main_tileset.exit_tile_id + m_map_data.first_gid; }

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
  std::ifstream load_json_file( std::filesystem::path json_file );
  void load_config( const std::filesystem::path &config_path );

  Data m_map_data;
  std::filesystem::path m_main_tileset_path{ "res/scenes/Tilesets/main.json" };
};

using SceneMapSharedPtr = std::shared_ptr<SceneData>;
using SceneMapWeakPtr = std::weak_ptr<SceneData>;

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SCENEDATA_HPP_