#ifndef SRC_FACTORY_PLAYERFACTORY_HPP
#define SRC_FACTORY_PLAYERFACTORY_HPP

#include <Components/Inventory/CarryItem.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

namespace ProceduralMaze::Factory
{

void CreatePlayer( entt::registry &registry );
entt::entity createWorldPosition( entt::registry &registry, const sf::Vector2f &pos );
entt::entity createVoidPosition( entt::registry &registry, const sf::Vector2f &pos );
void addSpawnArea( entt::registry &registry, entt::entity entity, float zorder );
void createPlayerDeathAnim( entt::registry &registry, Cmp::Position player_pos_cmp, const Sprites::MultiSprite &sprite );
entt::entity dropCarryItem( entt::registry &reg, Cmp::Position pos, const Sprites::MultiSprite &sprite, entt::entity inventory_slot_cmp_entt );
entt::entity pickupCarryItem( entt::registry &reg, entt::entity carryitem_entt );
entt::entity createCarryItem( entt::registry &reg, Cmp::Position pos, const Sprites::SpriteMetaType type );
void destroyInventory( entt::registry &reg, const Sprites::SpriteMetaType type );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_PLAYERFACTORY_HPP