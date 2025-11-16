#ifndef __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__
#define __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__

#include <entt/entity/registry.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>

#include <spdlog/spdlog.h>

#include <Components/Exit.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/Wall.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>

#include <optional>

namespace ProceduralMaze::Sys::ProcGen
{

class RandomLevelGenerator : public BaseSystem
{
public:
  RandomLevelGenerator( sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~RandomLevelGenerator() = default;

  void generate();

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void gen_positions();
  std::pair<entt::entity, Cmp::Position> find_spawn_location( unsigned long seed );
  void gen_large_obstacle( const Sprites::MultiSprite &large_obstacle_sprite, Sprites::SpriteMetaType, unsigned long seed );
  void gen_large_obstacles();

  // Iterate all position (excluding playable characters) and randomly assign small obstacles
  void gen_small_obstacles();

  void gen_loot_containers();
  void gen_npc_containers();

  // These obstacles are for the map border.
  // The textures are picked randomly, but their positions are fixed
  void gen_border();

  void add_wall_entity( const sf::Vector2f &pos, std::size_t sprite_index );
  void add_door_entity( const sf::Vector2f &pos, std::size_t sprite_index, bool is_exit );

  void stats();

  std::optional<entt::entity> at( std::size_t idx )
  {
    if ( idx > m_data.size() )
      return std::nullopt;
    else
      return m_data.at( idx );
  }
  auto data() { return m_data.data(); }
  auto begin() { return m_data.begin(); }
  auto end() { return m_data.end(); }
  auto size() { return m_data.size(); }

private:
  std::vector<entt::entity> m_data;
  Cmp::RandomInt m_activation_selector{ 0, 1 };
};

} // namespace ProceduralMaze::Sys::ProcGen

#endif // __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__