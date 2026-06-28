#ifndef SRC_FACTORY_RUINFACTORY_HPP__
#define SRC_FACTORY_RUINFACTORY_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

#include <Sprites/SpriteMetaType.hpp>
namespace Game::Sprites
{
class SpriteSheet;
class SpriteFactory;
} // namespace Game::Sprites

namespace Game::Factory
{

// void create_staircase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::SpriteSheet &stairs_ms );
void create_bookcase( entt::registry &reg, sf::Vector2f spawn_position, const Sprites::SpriteSheet &bookcase_ms, int sprite_index );
void create_cobweb( entt::registry &reg, entt::entity selected_entt, sf::Vector2f spawn_position, const Sprites::SpriteSheet &cobweb_ms,
                    int sprite_index );

//! @brief Create a shadow hand entt if none exists
//! @param reg
//! @param spawn_position
//! @param cobweb_ms
//! @param sprite_index
void create_shadow_hand( entt::registry &reg, sf::Vector2f scene_dimensions, const Sprites::SpriteSheet &hand_ms, int sprite_index = 0 );

} // namespace Game::Factory

#endif // SRC_FACTORY_RUINFACTORY_HPP__
