#include <Factory/ParticleFactory.hpp>
#include <Inventory/PlayerInventorySlot.hpp>
#include <Particle/ShockWave.hpp>
#include <Persistent/NpcShockwaveSpeed.hpp>
#include <Systems/PersistSystem.hpp>
#include <entt/entity/fwd.hpp>

namespace Game::Particle::Factory
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

void add_flame_for_player_inventory_slot( entt::registry &reg )
{
  // Add a flame ParticleSprite for a candle in the player inventory
  for ( auto [inventory_entt, inventory_cmp, inventory_uuid_cmp] : reg.view<Cmp::PlayerInventorySlot, Cmp::UUID>().each() )
  {
    if ( not inventory_cmp.m_item.sprite_type.contains( "candle" ) ) continue;
    Particle::Factory::add_flame( reg, "particle.candle", inventory_uuid_cmp, Utils::Player::get_position( reg ).getCenter(), 50000 );
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
  SPDLOG_INFO( "Created flame ParticleSprite {}", static_cast<uint32_t>( entt ) );
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
  SPDLOG_INFO( "Created smoke ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

void add_shockwave( entt::registry &reg, const std::string &tag, Cmp::UUID &uuid_cmp, sf::Vector2f pos, float zorder )
{
  auto ps = Cmp::Particle::ShockWave( 1000 );
  ps.set_tag( tag );
  ps.set_generations( 1 );
  ps.set_emitter_position( pos );
  ps.set_lifetime_ms( std::uniform_int_distribution<int>( 0, sf::seconds( 1 ).asMilliseconds() ) );
  ps.set_speed( Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveSpeed>( reg ).get_value() );
  ps.set_angle( std::uniform_real_distribution( 1.f, 360.f ) );

  auto entt = reg.create();
  reg.emplace_or_replace<Sys::ParticleSpriteOwner>( entt, Sys::ParticleSpriteOwner( std::make_unique<Cmp::Particle::ShockWave>( ps ) ) );
  reg.emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg.emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_INFO( "Created shockwave ParticleSprite {}", static_cast<uint32_t>( entt ) );
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

} // namespace Game::Particle::Factory