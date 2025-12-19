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
  enum class AreaShape { RECTANGLE, CIRCLE, CROSS };
  enum class SceneType { GRAVEYARD_EXTERIOR, CRYPT_INTERIOR };

  RandomLevelGenerator( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~RandomLevelGenerator() = default;

  void generate( AreaShape shape, sf::Vector2u map_grid_size, SceneType scene_type );

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  // Generate position components for the entire map grid and add player spawn
  void gen_rectangle_gamearea( sf::Vector2u map_grid_size );
  void gen_circular_gamearea( sf::Vector2u map_grid_size );

  //! @brief Generate a cross-shaped game area
  //! @param map_grid_size size of the total map grid in tiles
  //! @param armHalfWidth arm half-thickness in tiles (cross thickness = 2*armHalfWidth+1)
  //! @param vertHalfLengthModifier modifier for vertical arm half-length, i.e. 0.5 = half the map
  //! height
  //! @param horizHalfLengthModifier modifier for horizontal arm half-length, i.e. 0.25 = quarter
  //! the map width
  //! @param horizOffset move horizontal arm up by N tiles (positive pushes it downward)
  void gen_cross_gamearea( sf::Vector2u map_grid_size, int vertArmHalfWidth = 10, int horizArmHalfWidth = 5, int horizOffset = 10 );

  //! @brief create common obstacles (i.e. rock) for the graeyard
  void gen_graveyard_exterior_obstacles();
  
  //! @brief create multiblock sprites (i.e. graves, altars, crypts) for the graveyard scene.
  void gen_graveyard_exterior_multiblocks();

  //! @brief internal function for `gen_graveyard_exterior_multiblocks`
  //! @param ms
  //! @param seed
  void do_gen_graveyard_exterior_multiblock( const Sprites::MultiSprite &ms, unsigned long seed );

  //! @brief Generate the initial Crypt interior walls (fills in Cmp::CryptRoomsClosed)
  //! @note Except for start/end rooms, all other rooms should start as Cmp::CryptRoomsClosed
  void gen_crypt_initial_interior();

  //! @brief Generate the main objective for the Crypt
  //! @param map_grid_size
  void gen_crypt_main_objective( sf::Vector2u map_grid_size );

  //! @brief add multiblock sprites (pillars, etc...) to the Crypt
  void gen_crypt_interior_multiblocks();

  // Iterate and generate loot containers
  void gen_loot_containers( sf::Vector2u map_grid_size );

  // Iterate and generate npc containers
  void gen_npc_containers( sf::Vector2u map_grid_size );

  // Helper functions to add wall and door entities
  void add_wall_entity( const sf::Vector2f &pos, std::size_t sprite_index );

  // Find a valid spawn location for a large obstacle given a seed
  std::pair<entt::entity, Cmp::Position> find_spawn_location( const Sprites::MultiSprite &ms, unsigned long seed );

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
};

} // namespace ProceduralMaze::Sys::ProcGen

#endif // __SYSTEMS_PROCGEN_RANDOM_OBSTACLE_GENERATOR_SYSTEM_HPP__