#ifndef SRC_FACTORY_ACTIONFACTORY_HPP__
#define SRC_FACTORY_ACTIONFACTORY_HPP__

namespace Game::Factory::Action
{

void try_eat_inventory( entt::registry &reg );
void try_burn_worlditem( entt::registry &reg );

} // namespace Game::Factory::Action

#endif // SRC_FACTORY_ACTIONFACTORY_HPP__