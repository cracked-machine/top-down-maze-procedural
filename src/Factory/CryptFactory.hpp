#ifndef SRC_FACTORY_CRYPTFACTORY_HPP__
#define SRC_FACTORY_CRYPTFACTORY_HPP__

#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Wall.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Sprites/SpriteMetaType.hpp>

namespace Game::Sprites
{
class SpriteSheet;
class SpriteFactory;
} // namespace Game::Sprites

#include <entt/entity/fwd.hpp>

namespace Game::Crypt::Factory
{

entt::entity create_crypt_exit( entt::registry &reg, sf::Vector2f spawn_pos_px );
entt::entity create_crypt_lever( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder );
void destroy_crypt_lever( entt::registry &reg, entt::entity entt );

void create_crypt_lava_pit( entt::registry &reg, const Cmp::CryptRoomOpen &room,
                            std::shared_ptr<Game::PathFinding::SpatialHashGrid> pathfinding_navmesh );
void destroy_crypt_lava_pit( entt::registry &reg, entt::entity entt, std::shared_ptr<Game::PathFinding::SpatialHashGrid> pathfinding_navmesh );

void add_spike_trap( entt::registry &reg, const entt::entity entt, const int passage_id );

entt::entity create_crypt_chest( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder );
void destroy_crypt_chest( entt::registry &reg, entt::entity entt );

void create_crypt_shuffle_timer( entt::registry &reg, float threshold );
void destroy_crypt_shuffle_timer( entt::registry &reg );

} // namespace Game::Crypt::Factory

#endif // SRC_FACTORY_CRYPTFACTORY_HPP__