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
#include <Sprites/SpriteMetaType.hpp>

namespace ProceduralMaze::Sprites
{
class MultiSprite;
class SpriteFactory;
} // namespace ProceduralMaze::Sprites

#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Factory
{

//! @brief Create rooms that are NOT start/end rooms for the crypt game area.
//! @param reg
//! @param map_grid_size
void create_initial_crypt_rooms( entt::registry &reg, sf::Vector2u map_grid_size );

entt::entity CreateCryptLever( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder );
void DestroyCryptLever( entt::registry &reg, entt::entity entity );

void createCryptLavaPit( entt::registry &reg, const Cmp::CryptRoomOpen &room );
void destroyCryptLavaPit( entt::registry &reg, entt::entity entt );

void addSpikeTrap( entt::registry &reg, const entt::entity entt, const int passage_id );

entt::entity CreateCryptChest( entt::registry &reg, sf::Vector2f pos, Sprites::SpriteMetaType sprite_type, unsigned int sprite_idx, float zorder );
void DestroyCryptChest( entt::registry &reg, entt::entity entt );

//! @brief Generate the initial Crypt interior walls (fills in Cmp::CryptRoomsClosed)
//! @note Except for start/end rooms, all other rooms should start as Cmp::CryptRoomsClosed
void gen_crypt_initial_interior( entt::registry &reg, Sprites::SpriteFactory &sprite_factory );

//! @brief Generate the main objective for the Crypt
//! @param map_grid_size
void gen_crypt_main_objective( entt::registry &reg, Sprites::SpriteFactory &sprite_factory, sf::Vector2u map_grid_size );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_CRYPTFACTORY_HPP__