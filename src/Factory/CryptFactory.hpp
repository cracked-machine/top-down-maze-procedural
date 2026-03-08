#ifndef SRC_FACTORY_CRYPTFACTORY_HPP__
#define SRC_FACTORY_CRYPTFACTORY_HPP__

#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>

#include <Components/Neighbours.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Wall.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <SpatialHashGrid.hpp>
#include <Sprites/SpriteMetaType.hpp>

namespace ProceduralMaze::Sprites
{
class MultiSprite;
class SpriteFactory;
} // namespace ProceduralMaze::Sprites

#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Factory
{

entt::entity create_crypt_lever( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder );
void destroy_crypt_lever( entt::registry &reg, entt::entity entity );

void create_crypt_lava_pit( entt::registry &reg, const Cmp::CryptRoomOpen &room, PathFinding::SpatialHashGrid *spatial_grid );
void destroy_crypt_lava_pit( entt::registry &reg, entt::entity entt, PathFinding::SpatialHashGrid *spatial_grid );

void add_spike_trap( entt::registry &reg, const entt::entity entt, const int passage_id );

entt::entity create_crypt_chest( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder );
void destroy_crypt_chest( entt::registry &reg, entt::entity entt );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_CRYPTFACTORY_HPP__