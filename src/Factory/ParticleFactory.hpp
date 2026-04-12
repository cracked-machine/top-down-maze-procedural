#ifndef SRC_FACTORY_PARTICLEFACTORY_HPP_
#define SRC_FACTORY_PARTICLEFACTORY_HPP_

#include <Particle/Flame.hpp>
#include <Particle/ParticleSpriteTest.hpp>
#include <Particle/ShockWave.hpp>
#include <Particle/Smoke.hpp>
#include <Player.hpp>
#include <Systems/SystemStore.hpp>
namespace ProceduralMaze::Factory::Particle
{

void add_test( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag )
{
  auto psprite = Cmp::Particle::ParticleSpriteTest( 1000 );
  psprite.set_tag( tag );
  psprite.set_position( Utils::Player::get_position( reg ).getCenter() );
  psprite.set_lifetime( sf::seconds( 3 ) );
  psys.add( psprite );
}

void add_flame( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag )
{
  auto psprite = Cmp::Particle::Flame( 100 );
  psprite.set_tag( tag );
  psprite.set_position( Utils::Player::get_position( reg ).getCenter() );
  psprite.set_lifetime( sf::seconds( 0.5 ) );
  psys.add( psprite );
}

void add_smoke( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag )
{
  auto psprite = Cmp::Particle::Smoke( 100 );
  psprite.set_tag( tag );
  psprite.set_generations( 7 );
  psprite.set_position( Utils::Player::get_position( reg ).getCenter() );
  psprite.set_lifetime( sf::seconds( 1 ) );
  psys.add( psprite );
}

void add_shockwave( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag )
{
  auto psprite = Cmp::Particle::ShockWave( 1000 );
  psprite.set_tag( tag );
  psprite.set_generations( 1 );
  psprite.set_position( Utils::Player::get_position( reg ).getCenter() );
  psprite.set_lifetime( sf::seconds( 1 ) );
  psys.add( psprite );
}

} // namespace ProceduralMaze::Factory::Particle

#endif // SRC_FACTORY_PARTICLEFACTORY_HPP_