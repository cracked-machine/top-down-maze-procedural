#ifndef SRC_SYSTEMS_PROCGEN_LevelGenerator_HPP_
#define SRC_SYSTEMS_PROCGEN_LevelGenerator_HPP_

#include <PathFinding/SmartPointers.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>

// clang-format off
namespace Game::Cmp { class Position; class RectBounds; }
namespace sf { class RenderWindow; }
namespace Game::PathFinding { class SpatialHashGrid; } 
namespace Game::Sprites { class SpriteSheet; class SpriteFactory; }
namespace Game::Scene { class SceneData; }
// clang-format on

namespace Game::Sys::ProcGen
{

class LevelGenerator : public BaseSystem
{
public:
  //! @brief Used by level gen / cell automata
  enum class SceneType { GRAVEYARD_EXTERIOR, CRYPT_INTERIOR, HOLYWELL_INTERIOR };

  LevelGenerator( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );
  ~LevelGenerator() = default;

  //! @brief Generate game area using data from the SceneData object.
  //! @param scene_map
  void build_scene_from_data( const Scene::SceneData &scene_data );

  //! @brief create "sprite.graveyard.wall.int" for the graveyard.
  void gen_graveyard_exterior_obstacles();

  //! @brief
  void add_ruin_interior_obstacles();

  void add_ruin_rune_markers();

  //! @brief create multiblock sprites (i.e. graves, altars, crypts) for the graveyard scene.
  void gen_graveyard_exterior_multiblocks();

  //! @brief Find a valid spawn location for a large obstacle given a seed.
  //! @param ms
  //! @param seed
  //! @return std::pair<entt::entity, Cmp::Position>
  std::pair<entt::entity, Cmp::Position> find_spawn_location( const Sprites::SpriteSheet &ms, unsigned long seed );

  //! @brief Generate a number of plant world items in the new game area.
  //! @param map_grid_size
  //! @return std::vector<entt::entity>
  std::vector<entt::entity> gen_random_plants( sf::Vector2u map_grid_size );

  //! @brief Call this to make sure the level data is reset before regenerating a new scene
  void reset();

  //! @brief Get the obstacle sm object
  //! @return PathFinding::SpatialHashGrid&
  PathFinding::SpatialHashGrid &get_obstacle_sm();

  //! @brief Get the void sm object
  //! @return PathFinding::SpatialHashGrid&
  PathFinding::SpatialHashGrid &get_void_sm();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

private:
  //! @brief internal function for `gen_graveyard_exterior_multiblocks`.
  //! @param ms
  //! @param ms_index
  //! @param seed
  void do_gen_graveyard_exterior_multiblock( const Sprites::SpriteSheet &ms, size_t ms_index = 0, unsigned long seed = 0 );

  //! @brief Spatial map for finding obstacles during level gen / cell automata algorithm
  PathFinding::SpatialHashGridUniquePtr m_obstacle_sm;
  //! @brief Spatial map for marking void areas that are not part of the game area.
  PathFinding::SpatialHashGridUniquePtr m_void_sm;
};

} // namespace Game::Sys::ProcGen

#endif // SRC_SYSTEMS_PROCGEN_LevelGenerator_HPP_