#ifndef SRC_FACTORY_BOMB_FACTORY_HPP
#define SRC_FACTORY_BOMB_FACTORY_HPP

#include <Components/Armed.hpp>
#include <Components/Position.hpp>
#include <SFML/System/Time.hpp>
#include <Sprites/MultiSprite.hpp>
#include <entt/entt.hpp>
#include <optional>

namespace ProceduralMaze::Factory
{

entt::entity createArmed( entt::registry &registry, entt::entity entity, Cmp::Armed::EpiCenter epi_center, int sequence, int zorder );

void destroyArmed( entt::registry &reg, entt::entity armed_entity );

void createDetonated( entt::registry &reg, entt::entity armed_entity, Cmp::Position &armed_pos_cmp );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_BOMB_FACTORY_HPP