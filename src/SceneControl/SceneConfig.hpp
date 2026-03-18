#ifndef SRC_SCENECONTROL_SCENECONFIG_HPP_
#define SRC_SCENECONTROL_SCENECONFIG_HPP_

#include <Constants.hpp>
#include <memory>
namespace ProceduralMaze::Scene
{

class SceneConfig
{
public:
  struct Config
  {
    std::filesystem::path texture_path;
    sf::Vector2u tile_size;
    std::vector<uint32_t> floor_tile_pool;
    uint32_t random_seed;
    sf::Vector2u map_size;
    sf::Vector2u player_start_position;
    sf::Vector2u exit_position;
    std::multimap<std::string, sf::Vector2u> multiblock_positions;
    std::multimap<std::string, sf::Vector2u> npc_positions;
  };

  void load( const std::filesystem::path &config_path );

  std::filesystem::path get_texture_path() const { return m_config.texture_path; }
  const sf::Vector2u get_tile_size() const { return m_config.tile_size; }
  std::vector<uint32_t> get_floor_tile_pool() const { return m_config.floor_tile_pool; }
  uint32_t get_random_seed() const { return m_config.random_seed; }

  //! @brief  Get the map size as grid position and pixel position (x16)
  //!         For example, {50,70} and {800.0,1120.0}
  //! @return std::pair<sf::Vector2u, sf::Vector2f>
  std::pair<sf::Vector2u, sf::Vector2f> get_map_size() const;

  //! @brief  Get the player start position as grid position and pixel position (x16)
  //!         For example, {50,70} and {800.0,1120.0}
  //! @return std::pair<sf::Vector2u, sf::Vector2f>
  std::pair<sf::Vector2u, sf::Vector2f> get_player_start_position() const;

  //! @brief  Get the player exit position as grid position and pixel position (x16)
  //!         For example, {50,70} and {800.0,1120.0}
  //! @return std::pair<sf::Vector2u, sf::Vector2f>
  std::pair<sf::Vector2u, sf::Vector2f> get_exit_position() const;

  //! @brief Get a range of multiblock_positions as grid/pixel positions that match the key
  //! @param key
  //! @return std::vector<std::pair<sf::Vector2u, sf::Vector2f>>
  std::vector<std::pair<sf::Vector2u, sf::Vector2f>> get_sprite_position( std::string key );

  //! @brief Get a range of npc_positions as grid/pixel positions that match the key
  //! @param key
  //! @return std::vector<std::pair<sf::Vector2u, sf::Vector2f>>
  std::vector<std::pair<sf::Vector2u, sf::Vector2f>> get_npc_position( std::string key );

private:
  Config m_config;
};

using SceneConfigSharedPtr = std::shared_ptr<SceneConfig>;
using SceneConfigWeakPtr = std::weak_ptr<SceneConfig>;

} // namespace ProceduralMaze::Scene

#endif // SRC_SCENECONTROL_SCENECONFIG_HPP_