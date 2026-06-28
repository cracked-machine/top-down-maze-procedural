#ifndef SRC_SCENECONTROL_SCENEDATA_HPP__
#define SRC_SCENECONTROL_SCENEDATA_HPP__

#include <Sprites/SpriteMetaType.hpp>
#include <Utils/JsonDeserializer.hpp>
#include <nlohmann/json_fwd.hpp>

namespace Game::Scene
{

class SceneData : public Utils::JsonDeserializer
{
public:
  //! @brief External tileset used for levelgen layer.
  //!        The IDs are used by the level generator to determine the permitted placement types.
  struct MainTileSet
  {
    //! @brief The offset for the tile indexes within this tileset relative to the other tilesets.
    //! @note  This value is created and managed by Tiled application.
    int first_gid{ 0 };

    //! @brief Void cell
    int void_tile_id{ 0 };
    //! @brief Wall cell
    int wall_tile_id{ 0 };
    //! @brief Open cell
    int open_tile_id{ 0 };
    //! @brief Spawn cell
    int spawn_tile_id{ 0 };
    //! @brief Player cell
    int player_tile_id{ 0 };
    //! @brief Exit cell
    int exit_tile_id{ 0 };
    //! @brief Reserved cell
    int reserved_tile_id{ 0 };

    //! @brief Should be 16x16 pixels
    sf::Vector2u tile_size;
  };

  //! @brief The floor tileset
  struct FloorTileSet
  {
    //! @brief The png image file for the tileset.
    //! @note We have a specialised TileMap class for loading floor tilesets. i.e. not SpriteSheet.
    std::filesystem::path tileset_image;
    //! @brief List of selections to randomly choose from the tileset.
    //! @note  Duplicates will increase the odds of that index being picked.
    std::vector<int> tileset_pool;
    //! @brief Should be 16x16 pixels
    sf::Vector2u tile_size;
  };

  //! @brief The wall tileset
  struct WallTileSet
  {
    //! @brief The offset for the tile indexes within this tileset relative to the other tilesets.
    //! @note  This value is created and managed by Tiled application.
    int first_gid;
    //! @brief The SpriteSheet name. This must use a SpriteSheet defined in res/json/spritesheet_metadata.json.
    std::string name;
    //! @brief Should be 16x16 pixels
    sf::Vector2u tile_size;
  };

  // main data structure
  struct Data
  {
    //! @brief Overal game map size for the scene
    sf::Vector2u map_size;

    //! @brief tile index list for the levelgen layout
    std::vector<int> levelgen_tilelayer;
    //! @brief tile index list for the player layout
    std::vector<int> player_tilelayer;
    //! @brief tile index list for the wall layout
    std::vector<int> wall_tilelayer;
    //! @brief tile index list for the object layout
    std::vector<sf::FloatRect> solid_objectlayer;
    //! @brief tile index list for the mutliblock layout
    std::multimap<Sprites::SpriteMetaType, sf::Vector2f> multiblock_objectlayer;

    //! @brief Player start location
    sf::Vector2u player_start_position{ 0, 0 };

    //! @brief levelgen tileset
    MainTileSet main_tileset;
    //! @brief floor tileset
    FloorTileSet floor_tileset;
    //! @brief wall tileset
    WallTileSet wall_tileset;
  };

  SceneData( const std::filesystem::path &map_file );

  [[nodiscard]] int void_tile_id() const { return m_map_data.main_tileset.void_tile_id + m_map_data.main_tileset.first_gid; }
  [[nodiscard]] int wall_tile_id() const { return m_map_data.main_tileset.wall_tile_id + m_map_data.main_tileset.first_gid; }
  [[nodiscard]] int open_tile_id() const { return m_map_data.main_tileset.open_tile_id + m_map_data.main_tileset.first_gid; }
  [[nodiscard]] int spawn_tile_id() const { return m_map_data.main_tileset.spawn_tile_id + m_map_data.main_tileset.first_gid; }
  [[nodiscard]] int player_tile_id() const { return m_map_data.main_tileset.player_tile_id + m_map_data.main_tileset.first_gid; }
  [[nodiscard]] int exit_tile_id() const { return m_map_data.main_tileset.exit_tile_id + m_map_data.main_tileset.first_gid; }
  [[nodiscard]] int reserved_tile_id() const { return m_map_data.main_tileset.reserved_tile_id + m_map_data.main_tileset.first_gid; }

  [[nodiscard]] std::multimap<Sprites::SpriteMetaType, sf::Vector2f> multiblock_objectlayer() const { return m_map_data.multiblock_objectlayer; }
  [[nodiscard]] std::vector<sf::FloatRect> solid_objectlayer() const { return m_map_data.solid_objectlayer; }
  [[nodiscard]] std::vector<int> levelgen_tilelayer() const { return m_map_data.levelgen_tilelayer; }
  [[nodiscard]] std::vector<int> wall_tilelayer() const { return m_map_data.wall_tilelayer; }
  [[nodiscard]] std::filesystem::path floor_tileset_image() const { return m_map_data.floor_tileset.tileset_image; }
  [[nodiscard]] std::vector<int> floor_tileset_pool() const { return m_map_data.floor_tileset.tileset_pool; }
  [[nodiscard]] std::pair<sf::Vector2u, sf::Vector2f> get_player_start_position() const;
  [[nodiscard]] WallTileSet wall_tileset() const { return m_map_data.wall_tileset; }

  //! @brief  Get the map size as grid position and pixel position (x16)
  //!         For example, {50,70} and {800.0,1120.0}
  //! @return std::pair<sf::Vector2u, sf::Vector2f>
  [[nodiscard]] std::pair<sf::Vector2u, sf::Vector2f> map_size() const;

private:
  //! @brief Get the tilesets and tilelayers from the Tiled json file
  //! @param scene_tiledata_path
  void deserialize( const std::filesystem::path &json_scene_file_path );

  //! @brief Get the embedded floor tileset from the JSON objectp
  //! @param tileset The JSON input
  //! @param floor_tileset The output
  //! @return true
  //! @return false
  bool deserialize_int_floor_tileset( const nlohmann::json &json );

  //! @brief Get the embedded wall tileset from the JSON object
  //! @param tileset The JSON input
  //! @param wall_first_gid The output
  //! @return true
  //! @return false
  bool deserialize_int_wall_tileset( const nlohmann::json &json );

  //! @brief Get the external main tileset from the JSON object
  //! @param scene_tilemap_path Used for meaningful logging
  //! @param tileset The JSON input
  //! @param main_tileset The output
  //! @param main_ext_first_gid The output
  //! @return true
  //! @return false
  bool deserialize_ext_main_tileset( const std::filesystem::path &scene_tilemap_path, const nlohmann::json &tileset );

  //! @brief Deserialize the scene tilemap json and copy in 'floor_tileset' and 'main_tileset'
  //! @param scene_tilemap_path Used for meaningful logging
  //! @param scene_tilemap_json The JSON input
  //! @param scene_tilemap The output
  void deserialize_tilelayers( const nlohmann::json &scene_tilemap_json );

  //! @brief Get the player start position from player_tilelayer
  //! @param scene_tilemap
  void retrieve_player_start_pos();

  Data m_map_data;
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENEDATA_HPP__
