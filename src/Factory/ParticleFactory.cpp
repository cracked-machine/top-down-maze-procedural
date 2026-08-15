#include <Components/Inventory/PlayerInventorySlot.hpp>
#include <Components/Particle/CryptAltarParticleSprite.hpp>
#include <Components/Particle/ObstacleDigParticleSprite.hpp>
#include <Components/Particle/RuneParticleSprite.hpp>
#include <Components/Particle/ShockWave.hpp>
#include <Components/Particle/SpriteTest.hpp>
#include <Components/Particle/WatchmanGunfireParticleSprite.hpp>
#include <Components/Particle/WormholeParticleSprite.hpp>
#include <Components/Persistent/NpcShockwaveSpeed.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Factory/ParticleFactory.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Time.hpp>
#include <Systems/PersistSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Player.hpp>

#include <entt/entity/fwd.hpp>

namespace Game::Factory::Particle
{

void add_test( entt::registry &reg, Sys::ParticleSystem &psys, const std::string &tag )
{
  auto psprite = Cmp::Particle::SpriteTest( 1000 );
  psprite.set_tag( tag );
  psprite.set_emitter_position( Utils::Player::get_position( reg ).getCenter() );
  psprite.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( 3 ).asMilliseconds() ) );
  psprite.set_speed( std::uniform_real_distribution<float>( 1.f, 100.f ) );
  psprite.set_angle( std::uniform_real_distribution<float>( 1.f, 360.f ) );
  psys.add( std::make_pair( psprite, Cmp::ZOrderValue( 10000.f ) ) );
}

void add_crypt_altar_ps( entt::registry &reg, const std::string &tag, float lifetime_seconds, float speed, Cmp::UUID &uuid_cmp, sf::Vector2f pos,
                         float zorder )
{
  auto ps = Cmp::Particle::CryptAltarParticleSprite( 1000 );
  ps.set_tag( tag );
  ps.set_emitter_position( pos );
  ps.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( lifetime_seconds ).asMilliseconds() ) );
  ps.set_speed( std::uniform_real_distribution<float>( 1.f, speed ) );
  ps.set_angle( std::uniform_real_distribution<float>( 1.f, 360.f ) );

  auto entt = reg.create();
  reg.emplace_or_replace<Sys::ParticleSpriteOwner>( entt,
                                                    Sys::ParticleSpriteOwner( std::make_unique<Cmp::Particle::CryptAltarParticleSprite>( ps ) ) );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_DEBUG( "Created flame ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void add_watchman_gunfire_ps( entt::registry &reg, const std::string &tag, float lifetime_seconds, float speed, Cmp::UUID &uuid_cmp, sf::Vector2f pos,
                              float zorder )
{
  auto ps = Cmp::Particle::WatchmanGunfireParticleSprite( 3 );
  ps.set_tag( tag );
  ps.set_generations( 1 );
  ps.set_emitter_position( pos );
  ps.set_lifetime_ms(
      std::uniform_int_distribution<int>( sf::seconds( lifetime_seconds ).asMilliseconds(), sf::seconds( lifetime_seconds ).asMilliseconds() ) );
  ps.set_speed( std::uniform_real_distribution<float>( speed, speed ) );

  // aim the gunfire at the player, with a small random spread rather than a full 360-degree burst
  // guard against a zero-length vector (player and watchman occupying the same point, e.g. after a
  // knockback), which would otherwise trip SFML's Vector2::angle() assertion
  const sf::Vector2f direction = Utils::Player::get_position( reg ).getCenter() - pos;
  const float base_angle_degrees = Utils::Maths::angle( direction ).value_or( sf::Angle::Zero ).asDegrees();
  constexpr float kSpreadDegrees = 16.f;
  ps.set_angle( std::uniform_real_distribution<float>( base_angle_degrees - kSpreadDegrees, base_angle_degrees + kSpreadDegrees ) );

  auto entt = reg.create();
  reg.emplace_or_replace<Sys::ParticleSpriteOwner>(
      entt, Sys::ParticleSpriteOwner( std::make_unique<Cmp::Particle::WatchmanGunfireParticleSprite>( ps ) ) );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_DEBUG( "Created gunfire ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void add_rune_ps( entt::registry &reg, const std::string &tag, float lifetime_seconds, float speed, Cmp::UUID &uuid_cmp, sf::Vector2f pos,
                  float zorder )
{
  auto ps = Cmp::Particle::RuneParticleSprite( 1000 );
  ps.set_tag( tag );
  ps.set_emitter_position( pos );
  ps.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( lifetime_seconds ).asMilliseconds() ) );
  ps.set_speed( std::uniform_real_distribution<float>( 1.f, speed ) );
  ps.set_angle( std::uniform_real_distribution<float>( 1.f, 360.f ) );

  auto entt = reg.create();
  reg.emplace_or_replace<Sys::ParticleSpriteOwner>( entt, Sys::ParticleSpriteOwner( std::make_unique<Cmp::Particle::RuneParticleSprite>( ps ) ) );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_DEBUG( "Created rune ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void add_wormhole_ps( entt::registry &reg, const std::string &tag, float lifetime_seconds, float speed, Cmp::UUID &uuid_cmp, sf::Vector2f pos,
                      float zorder )
{
  auto ps = Cmp::Particle::WormholeParticleSprite( 1000 );
  ps.set_tag( tag );
  ps.set_emitter_position( pos );
  ps.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( lifetime_seconds ).asMilliseconds() ) );
  ps.set_speed( std::uniform_real_distribution<float>( 1.f, speed ) );
  ps.set_angle( std::uniform_real_distribution<float>( 1.f, 360.f ) );

  auto entt = reg.create();
  reg.emplace_or_replace<Sys::ParticleSpriteOwner>( entt, Sys::ParticleSpriteOwner( std::make_unique<Cmp::Particle::WormholeParticleSprite>( ps ) ) );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_DEBUG( "Created wormhole ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void add_obstacledig_ps( entt::registry &reg, const std::string &tag, int particle_count, float lifetime_seconds, float speed, Cmp::UUID &uuid_cmp,
                         sf::Vector2f pos, float zorder )
{
  auto ps = Cmp::Particle::ObstacleDigParticleSprite( particle_count );
  ps.set_tag( tag );
  ps.set_generations( 1 );

  ps.set_emitter_position( { pos.x + Cmp::RandomFloat( 4.f, 12.f ).gen(), pos.y + Cmp::RandomFloat( 4.f, 12.f ).gen() } );

  ps.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( lifetime_seconds ).asMilliseconds() ) );
  ps.set_speed( std::uniform_real_distribution<float>( speed, speed ) );
  ps.set_angle( std::uniform_real_distribution<float>( 1.f, 360.f ) );

  auto entt = reg.create();
  reg.emplace_or_replace<Sys::ParticleSpriteOwner>( entt,
                                                    Sys::ParticleSpriteOwner( std::make_unique<Cmp::Particle::ObstacleDigParticleSprite>( ps ) ) );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_DEBUG( "Created obstacle ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void add_flame_for_player_inventory_slot( entt::registry &reg )
{
  // Add a flame ParticleSprite for a candle in the player inventory
  for ( auto [inventory_entt, inventory_cmp, inventory_uuid_cmp] : reg.view<Cmp::PlayerInventorySlot, Cmp::UUID>().each() )
  {
    if ( not inventory_cmp.m_item.sprite_type.contains( "candle" ) ) continue;
    Factory::Particle::add_flame( reg, "particle.candle", inventory_uuid_cmp, Utils::Player::get_position( reg ).getCenter(), 50000 );
    for ( auto [ps_entt, ps_owner, ps_uuid_cmp] : reg.view<Sys::ParticleSpriteOwner, Cmp::UUID>().each() )
    {
      if ( ps_uuid_cmp != inventory_uuid_cmp ) continue;

      // Move the ParticleSprite to the UI view. Any particle sprites should be fully cleared
      // otherwise we get particle effects in strange places during the transition frame.
      // The emitter position is set by RenderGameSystem using the UiData object.
      ps_owner.sprite->clear();
      ps_owner.sprite->set_view_type( Cmp::Particle::ViewType::SCREEN );
    }
  }
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
  SPDLOG_DEBUG( "Created flame ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void add_smoke( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder )
{
  auto ps = Cmp::Particle::Smoke( 100 );
  ps.set_tag( tag );
  ps.set_generations( 7 );
  ps.set_emitter_position( pos );
  ps.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( 0.5 ).asMilliseconds() ) );
  ps.set_speed( 20.f );
  ps.set_phase( std::uniform_real_distribution( 0.f, 2.f * std::numbers::pi_v<float> ) );
  ps.set_freq( std::uniform_real_distribution( 0.3f, 0.8f ) );

  auto entt = reg.create();
  reg.emplace_or_replace<Sys::ParticleSpriteOwner>( entt, Sys::ParticleSpriteOwner( std::make_unique<Cmp::Particle::Smoke>( ps ) ) );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_DEBUG( "Created smoke ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void add_shockwave( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder )
{
  // don't set the angle here, the particles are assigned an angle in the ShockWave() constructor
  auto ps = Cmp::Particle::ShockWave( 1000 );
  ps.set_tag( tag );
  ps.set_generations( 1 );
  ps.set_emitter_position( pos );
  ps.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( 1 ).asMilliseconds() ) );
  ps.set_speed( Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveSpeed>( reg ).get_value() );

  auto entt = reg.create();
  reg.emplace_or_replace<Sys::ParticleSpriteOwner>( entt, Sys::ParticleSpriteOwner( std::make_unique<Cmp::Particle::ShockWave>( ps ) ) );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_DEBUG( "Created shockwave ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void delete_expired_particle_sprites( entt::registry &reg, const std::string &search_pattern )
{
  // remove the particle sprite once it has stopped
  for ( auto [ps_entt, ps_cmp] : reg.view<Sys::ParticleSpriteOwner>().each() )
  {
    if ( not ps_cmp.sprite->get_tag().contains( search_pattern ) ) continue;
    if ( ps_cmp.sprite->is_active() ) continue;
    if ( not reg.valid( ps_entt ) ) continue;
    reg.destroy( ps_entt );
  }
}

void update_position( entt::registry &reg, const std::string &search_pattern, sf::Vector2f pos )
{
  // update the position so that it follows player
  for ( auto [smoke_entt, smoke_ps_cmp] : reg.view<Sys::ParticleSpriteOwner>().each() )
  {
    if ( not smoke_ps_cmp.sprite->get_tag().contains( search_pattern ) ) continue;
    smoke_ps_cmp.sprite->set_emitter_position( pos );
  }
}

void update_position( entt::registry &reg, Cmp::UUID uuid_cmp, sf::Vector2f pos )
{
  // update the position so that it follows player
  for ( auto [smoke_entt, smoke_ps_cmp, ps_uuid_cmp] : reg.view<Sys::ParticleSpriteOwner, Cmp::UUID>().each() )
  {
    if ( ps_uuid_cmp != uuid_cmp ) continue;
    smoke_ps_cmp.sprite->set_emitter_position( pos );
  }
}

} // namespace Game::Factory::Particle