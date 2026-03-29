#ifndef SRC_FACTORY_ALTARFACTORY_HPP_
#define SRC_FACTORY_ALTARFACTORY_HPP_

#include <Components/Position.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Factory
{

void create_altar_sacrifice_anim( entt::registry &reg, Cmp::Position pos, Sprites::SpriteMetaType anim_type );

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_ALTARFACTORY_HPP_