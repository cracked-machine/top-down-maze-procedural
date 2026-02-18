#ifndef SRC_FACTORY_PLAYERFACTORY_HPP
#define SRC_FACTORY_PLAYERFACTORY_HPP

#include <Components/Inventory/CarryItem.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/fwd.hpp>

namespace ProceduralMaze::Factory
{

void CreatePlayer( entt::registry &registry );
entt::entity createWorldPosition( entt::registry &registry, const sf::Vector2f &pos );
entt::entity createVoidPosition( entt::registry &registry, const sf::Vector2f &pos );
void addSpawnArea( entt::registry &registry, entt::entity entity, float zorder );
void createPlayerDeathAnim( entt::registry &registry, Cmp::Position player_pos_cmp, const Sprites::MultiSprite &sprite );
entt::entity dropInventorySlotIntoWorld( entt::registry &reg, Cmp::Position pos, const Sprites::MultiSprite &sprite,
                                         entt::entity inventory_slot_cmp_entt );
entt::entity pickupCarryItem( entt::registry &reg, entt::entity carryitem_entt );
entt::entity createCarryItem( entt::registry &reg, Cmp::Position pos, const Sprites::SpriteMetaType type, float zorder = 0.f );
void destroyInventory( entt::registry &reg, const Sprites::SpriteMetaType type );

Cmp::Position add_player_last_graveyard_pos( entt::registry &reg, Cmp::Position &last_known_pos,
                                             sf::Vector2f offset = { 0.f, Constants::kGridSizePxF.y } );
void remove_player_last_graveyard_pos( entt::registry &reg );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_PLAYERFACTORY_HPP