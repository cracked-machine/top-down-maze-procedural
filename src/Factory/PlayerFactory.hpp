#ifndef SRC_FACTORY_PLAYERFACTORY_HPP
#define SRC_FACTORY_PLAYERFACTORY_HPP

#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/fwd.hpp>

namespace ProceduralMaze::Factory
{

void create_player( entt::registry &registry );

void add_spawn_area( entt::registry &registry, entt::entity entity, Sprites::SpriteFactory &sfactory, float zorder );
void create_player_death_anim( entt::registry &registry, Cmp::Position player_pos_cmp, const Sprites::MultiSprite &sprite );

entt::entity pickup_world_item( entt::registry &reg, entt::entity world_item_entt );

entt::entity create_world_item( entt::registry &reg, Cmp::Position pos, const std::string &item, float zorder = 0.f );

void destroy_inventory( entt::registry &reg, const Sprites::SpriteMetaType &type );

//! @brief Destroy all player inventory slots matching a type. See "sprite.item.xxxx" in res/json/sprite_metadata.json
//! @param reg the ECS registry
//! @param type the type to destroy
void add_inventory( entt::registry &reg, const std::string &item );
Cmp::Position add_player_last_graveyard_pos( entt::registry &reg, Cmp::Position &last_known_pos,
                                             sf::Vector2f offset = { 0.f, Constants::kGridSizePxF.y } );
void remove_player_last_graveyard_pos( entt::registry &reg );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_PLAYERFACTORY_HPP