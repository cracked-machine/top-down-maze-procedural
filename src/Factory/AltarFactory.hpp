#ifndef SRC_FACTORY_ALTARFACTORY_HPP__
#define SRC_FACTORY_ALTARFACTORY_HPP__

#include <Components/Position.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <entt/entity/fwd.hpp>

namespace Game::Factory::Altar
{

void create_altar_sacrifice_anim( entt::registry &reg, Cmp::Position pos, Sprites::SpriteMetaType anim_type );

} // namespace Game::Factory::Altar

#endif // SRC_FACTORY_ALTARFACTORY_HPP__
