#ifndef SRC_FACTORY_BOMBFACTORY_HPP__
#define SRC_FACTORY_BOMBFACTORY_HPP__

#include <Components/Armed.hpp>
#include <Components/Position.hpp>
#include <SFML/System/Time.hpp>
#include <entt/fwd.hpp>

namespace Game::Factory::Bomb
{

entt::entity create_armed( entt::registry &registry, entt::entity entity, Cmp::Armed::EpiCenter epi_center, int sequence, int zorder );

void destroy_armed( entt::registry &reg, entt::entity armed_entity );

void add_detonated( entt::registry &reg, entt::entity armed_entity, Cmp::Position &armed_pos_cmp );

} // namespace Game::Factory::Bomb

#endif // SRC_FACTORY_BOMBFACTORY_HPP__
