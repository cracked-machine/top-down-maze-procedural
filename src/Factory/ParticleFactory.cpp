#include <Factory/ParticleFactory.hpp>

namespace ProceduralMaze::Factory::Particle
{

void add_test( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag )
{
  auto psprite = Cmp::Particle::ParticleSpriteTest( 1000 );
  psprite.set_tag( tag );
  psprite.set_emitter_position( Utils::Player::get_position( reg ).getCenter() );
  psprite.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( 3 ).asMilliseconds() ) );
  psprite.set_speed( std::uniform_real_distribution<float>( 1.f, 100.f ) );
  psprite.set_angle( std::uniform_real_distribution<float>( 1.f, 360.f ) );
  psys.add( std::make_pair( psprite, Cmp::ZOrderValue( 10000.f ) ) );
}

void add_flame( [[maybe_unused]] entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos )
{
  auto psprite = Cmp::Particle::Flame( 100 );
  psprite.set_tag( tag );
  psprite.set_emitter_position( pos );
  psprite.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( 0.5 ).asMilliseconds() ) );
  psprite.set_speed( 40.f );
  psprite.set_phase( std::uniform_real_distribution( 0.f, 2.f * std::numbers::pi_v<float> ) );
  psprite.set_freq( std::uniform_real_distribution( 0.3f, 0.8f ) );

  psys.add( uuid_cmp, psprite, Cmp::ZOrderValue( 50000.f ) );
}

void add_flame( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder )
{

  auto ps = Cmp::Particle::Flame( 100 );
  ps.set_tag( tag );
  ps.set_emitter_position( pos );
  ps.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( 0.5 ).asMilliseconds() ) );
  ps.set_speed( 40.f );
  ps.set_phase( std::uniform_real_distribution( 0.f, 2.f * std::numbers::pi_v<float> ) );
  ps.set_freq( std::uniform_real_distribution( 0.3f, 0.8f ) );

  auto entt = reg.create();
  reg.emplace_or_replace<Sys::ParticleSpriteOwner>( entt, Sys::ParticleSpriteOwner( std::make_unique<Cmp::Particle::Flame>( ps ) ) );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_INFO( "Created ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void add_smoke( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag )
{
  auto psprite = Cmp::Particle::Smoke( 100 );
  psprite.set_tag( tag );
  psprite.set_generations( 7 );
  psprite.set_emitter_position( Utils::Player::get_position( reg ).getCenter() );
  psprite.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( 1 ).asMilliseconds() ) );
  psprite.set_speed( 20.f );
  psprite.set_phase( std::uniform_real_distribution( 0.f, 2.f * std::numbers::pi_v<float> ) );
  psprite.set_freq( std::uniform_real_distribution( 0.3f, 0.8f ) );

  psys.add( std::make_pair( psprite, Cmp::ZOrderValue( 10000.f ) ) );
}

void add_shockwave( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag )
{
  auto psprite = Cmp::Particle::ShockWave( 1000 );
  psprite.set_tag( tag );
  psprite.set_generations( 1 );
  psprite.set_emitter_position( Utils::Player::get_position( reg ).getCenter() );
  psprite.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( 1 ).asMilliseconds() ) );
  psprite.set_speed( 100.f );
  psprite.set_angle( std::uniform_real_distribution<float>( 1.f, 360.f ) );
  psys.add( std::make_pair( psprite, Cmp::ZOrderValue( 10000.f ) ) );
}

} // namespace ProceduralMaze::Factory::Particle