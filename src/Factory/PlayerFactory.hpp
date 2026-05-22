#ifndef SRC_FACTORY_PLAYERFACTORY_HPP
#define SRC_FACTORY_PLAYERFACTORY_HPP

#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Factory/SpriteFactory.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/fwd.hpp>

namespace Game::Factory
{

void create_player( entt::registry &registry );

void add_spawn_area( entt::registry &registry, entt::entity entity, Sprites::SpriteFactory &sfactory, float zorder );
void create_player_death_anim( entt::registry &registry, Cmp::Position player_pos_cmp, const Sprites::SpriteSheet &sprite );

void destroy_inventory( entt::registry &reg, const Sprites::SpriteMetaType &type );

//! @brief Destroy all player inventory slots matching a type. See "sprite.item.xxxx" in res/json/sprite_metadata.json
//! @param reg the ECS registry
//! @param type the type to destroy
void add_inventory( entt::registry &reg, const std::string &item );
Cmp::Position add_player_last_graveyard_pos( entt::registry &reg, Cmp::Position &last_known_pos,
                                             sf::Vector2f offset = { 0.f, Constants::kGridSizePxF.y } );
void remove_player_last_graveyard_pos( entt::registry &reg );

} // namespace Game::Factory

#endif // SRC_FACTORY_PLAYERFACTORY_HPP