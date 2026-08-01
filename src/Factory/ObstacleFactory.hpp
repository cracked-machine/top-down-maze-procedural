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

namespace Game::Factory
{

entt::entity create_world_pos( entt::registry &registry, const sf::Vector2f &pos );
entt::entity create_void_pos( entt::registry &registry, const Cmp::Position &pos );

//! @brief Create an obstacle WITHOUT sprite for procedural generation algorithm.
//! @param registry
//! @param entity
//! @param reserved_navmesh If provided, skips placement when the position is already reserved (O(1) check).
//! @return true if the obstacle was placed, false if blocked by the reserved navmesh.
bool add_obstacle( entt::registry &registry, entt::entity entity, const PathFinding::SpatialHashGridSharedPtr &reserved_navmesh = nullptr );

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
enum class DeleteExtras : bool { No = false, Yes = true };

//! @brief Remove the obstacle component from the entity
//! @param registry
//! @param obstacle_entity
//! @param delete_extras Delete any extra obstacles entitys found (doesn't delete the search_entt)
void remove_obstacle( entt::registry &reg, entt::entity search_entt, DeleteExtras delete_extras = DeleteExtras::No );

using UUIDEntityMap = std::unordered_map<Cmp::UUID, entt::entity>;
void remove_obstacle( entt::registry &reg, entt::entity search_entt, DeleteExtras delete_extras, const UUIDEntityMap &uuid_map );

} // namespace Game::Factory

#endif // SRC_FACTORY_OBSTACLEFACTORY_HPP__
