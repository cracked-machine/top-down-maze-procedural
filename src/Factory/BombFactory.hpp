#ifndef SRC_FACTORY_BOMB_FACTORY_HPP
#define SRC_FACTORY_BOMB_FACTORY_HPP

#include <Components/Armed.hpp>
#include <Components/Position.hpp>
#include <SFML/System/Time.hpp>
#include <entt/fwd.hpp>

namespace ProceduralMaze::Factory
{

entt::entity create_armed( entt::registry &registry, entt::entity entity, Cmp::Armed::EpiCenter epi_center, int sequence, int zorder );

void destroy_armed( entt::registry &reg, entt::entity armed_entity );

void create_detonated( entt::registry &reg, entt::entity armed_entity, Cmp::Position &armed_pos_cmp );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_BOMB_FACTORY_HPP