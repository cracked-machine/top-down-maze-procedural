#ifndef SRC_FACTORY_PLAYERFACTORY_HPP
#define SRC_FACTORY_PLAYERFACTORY_HPP

#include <Components/Inventory/CarryItem.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/fwd.hpp>

namespace ProceduralMaze::Factory
{

void create_player( entt::registry &registry );

void add_spawn_area( entt::registry &registry, entt::entity entity, float zorder );
void create_player_death_anim( entt::registry &registry, Cmp::Position player_pos_cmp, const Sprites::MultiSprite &sprite );
entt::entity drop_inventory_slot_into_world( entt::registry &reg, Cmp::Position pos, const Sprites::MultiSprite &sprite,
                                             entt::entity inventory_slot_cmp_entt );
entt::entity pickup_carry_item( entt::registry &reg, entt::entity carryitem_entt );
entt::entity create_carry_item( entt::registry &reg, Cmp::Position pos, const Sprites::SpriteMetaType type, float zorder = 0.f );
void destroy_inventory( entt::registry &reg, const Sprites::SpriteMetaType type );
void add_inventory( entt::registry &reg, Sprites::SpriteMetaType item );
Cmp::Position add_player_last_graveyard_pos( entt::registry &reg, Cmp::Position &last_known_pos,
                                             sf::Vector2f offset = { 0.f, Constants::kGridSizePxF.y } );
void remove_player_last_graveyard_pos( entt::registry &reg );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_PLAYERFACTORY_HPP