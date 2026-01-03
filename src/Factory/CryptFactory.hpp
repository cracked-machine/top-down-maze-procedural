#ifndef SRC_FACTORY_CRYPTFACTORY_HPP__
#define SRC_FACTORY_CRYPTFACTORY_HPP__

#include <Components/CryptRoomClosed.hpp>
#include <Components/CryptRoomEnd.hpp>
#include <Components/CryptRoomOpen.hpp>
#include <Components/CryptRoomStart.hpp>

#include <Components/Neighbours.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Wall.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

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

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_CRYPTFACTORY_HPP__