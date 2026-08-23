#ifndef SRC_FACTORY_OBSTACLEFACTORY_HPP__
#define SRC_FACTORY_OBSTACLEFACTORY_HPP__

#include <Components/Position.hpp>
#include <Components/UUID.hpp>
#include <PathFinding/SmartPointers.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <entt/fwd.hpp>
#include <unordered_map>

// clang-format off
namespace Game::Sprites { class SpriteSheet; class SpriteFactory; }
// clang-format on

namespace Game::Factory::Obstacle
{

//! @brief Create a bare, grid-sized world position entity that other factories can attach obstacles/sprites to.
//! @param registry
//! @param pos
//! @return The new entity.
entt::entity create_world_pos( entt::registry &registry, const sf::Vector2f &pos );

//! @brief Create a VoidPosition entity, marking a grid cell as outside the playable world.
//! @param registry
//! @param pos
//! @return The new entity.
entt::entity create_void_pos( entt::registry &registry, const Cmp::Position &pos );

//! @brief Create an obstacle WITHOUT sprite for procedural generation algorithm.
//! @param registry
//! @param entity
//! @param reserved_navmesh If provided, skips placement when the position is already reserved (O(1) check).
//! @return true if the obstacle was placed, false if blocked by the reserved navmesh.
bool add_obstacle( entt::registry &registry, entt::entity entity, const PathFinding::SpatialHashGridSharedPtr &reserved_navmesh = nullptr );

//! @brief Mark the entity as the "cap" sprite paired with an Obstacle entity (tied together via Cmp::UUID)
//! @param registry
//! @param entity
void add_obstacle_cap( entt::registry &registry, entt::entity entity );

//! @brief Create an obstacle WITH sprite for final level gen pass
//! @param registry
//! @param entity
//! @param pos_cmp
//! @param ms
//! @param sprite_tile_idx
//! @param zorder
//! @param blocking
void decorate_obstacle( entt::registry &registry, entt::entity entity, Cmp::Position pos_cmp, const Sprites::SpriteSheet &ms,
                        std::size_t sprite_tile_idx, float zorder = 0, bool blocking = true );

//! @brief Whether remove_obstacle() should also destroy the matching cap entity (tied by UUID).
enum class DeleteExtras : bool
{
  //! @brief Do not delete the matching cap entity.
  No = false,
  //! @brief Also delete the matching cap entity.
  Yes = true
};

//! @brief Remove the obstacle component from the entity
//! @param registry
//! @param obstacle_entity
//! @param delete_extras Delete any extra obstacles entitys found (doesn't delete the search_entt)
void remove_obstacle( entt::registry &reg, entt::entity search_entt, DeleteExtras delete_extras = DeleteExtras::No );

//! @brief Map of obstacle UUID to its cap entity, built ahead of time for O(1) lookup during removal.
using UUIDEntityMap = std::unordered_map<Cmp::UUID, entt::entity>;

//! @brief Remove the obstacle component from the entity, using `uuid_map` for an O(1) cap entity lookup.
//! @param reg
//! @param search_entt
//! @param delete_extras Delete any extra obstacle entities found (doesn't delete `search_entt`).
//! @param uuid_map Map of UUID to cap entity, built ahead of time to avoid a full registry scan per call.
void remove_obstacle( entt::registry &reg, entt::entity search_entt, DeleteExtras delete_extras, const UUIDEntityMap &uuid_map );

} // namespace Game::Factory::Obstacle

#endif // SRC_FACTORY_OBSTACLEFACTORY_HPP__
