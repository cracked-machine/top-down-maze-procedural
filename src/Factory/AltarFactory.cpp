#include <Components/Altar/AltarSacrifice.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/AltarFactory.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Factory
{

void createAltarSacrificeAnimation( entt::registry &reg, Cmp::Position pos )
{
  entt::entity entt = reg.create();
  reg.emplace_or_replace<Cmp::AltarSacrifice>( entt );
  reg.emplace_or_replace<Cmp::Position>( entt, pos.position, pos.size );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( entt, 0, 0, true, "ALTAR.sacrifice.anim", 0 );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, pos.position.y * 4.f );
}

} // namespace ProceduralMaze::Factory
