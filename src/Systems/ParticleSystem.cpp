
#include <Inventory/WorldItem.hpp>
#include <Optimizations.hpp>
#include <Position.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <UUID.hpp>
#include <ZOrderValue.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Sys
{

ParticleSystem::ParticleSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

void ParticleSystem::add_to_registry( Cmp::UUID &uuid_cmp, ParticleSpriteOwner owner, Cmp::ZOrderValue zorder )
{

  auto entt = reg().create();
  reg().emplace_or_replace<ParticleSpriteOwner>( entt, std::move( owner ) );
  reg().emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  reg().emplace_or_replace<Cmp::UUID>( entt, uuid_cmp.data );
  SPDLOG_INFO( "Created ParticleSprite {}", static_cast<uint32_t>( entt ) );
}

std::vector<entt::entity> ParticleSystem::add_to_registry( ParticleSpriteOwner owner, Cmp::ZOrderValue zorder )
{
  std::vector<entt::entity> entt_list;

  auto entt = reg().create();
  reg().emplace_or_replace<ParticleSpriteOwner>( entt, std::move( owner ) );
  reg().emplace_or_replace<Cmp::ZOrderValue>( entt, zorder );
  entt_list.push_back( entt );
  SPDLOG_INFO( "Created ParticleSprite {}", static_cast<uint32_t>( entt ) );

  return entt_list;
}

void ParticleSystem::update( sf::Time dt )
{
  for ( auto [ps_entt, ps_owner, ps_uuid_cmp] : reg().view<Sys::ParticleSpriteOwner, Cmp::UUID>().each() )
  {
    for ( auto [candle_entt, candle_cmp, candle_pos_cmp, candle_uuid_cmp] : reg().view<Cmp::WorldItem, Cmp::Position, Cmp::UUID>().each() )
    {
      if ( not candle_cmp.sprite_type.contains( "candle" ) ) continue;
      if ( ps_uuid_cmp != candle_uuid_cmp ) continue;
      ps_owner.sprite->set_emitter_position( candle_pos_cmp.getCenter() );
    }
  }

  for ( auto [entt, owner] : reg().view<ParticleSpriteOwner>().each() )
  {
    if ( not owner.sprite->is_active() ) continue;

    owner.sprite->simulate( dt );
    owner.sprite->prune_inactive_expired_particles();
    owner.sprite->deactivate_extinct_particles();
  }
}

void ParticleSystem::check_collsion( const sf::FloatRect &target )
{
  if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), target ) ) return;
  for ( auto [entt, owner] : reg().view<ParticleSpriteOwner>().each() )
  {
    if ( owner.sprite->is_active() )
    {
      SPDLOG_DEBUG( "Simulating" );
      owner.sprite->check_particle_collision( target );
    }
  }
}

[[nodiscard]] std::vector<std::reference_wrapper<Cmp::Particle::IParticleSprite>> ParticleSystem::find( entt::registry &reg, const std::string &tag )
{
  std::vector<std::reference_wrapper<Cmp::Particle::IParticleSprite>> particle_sprites;
  for ( auto [entt, owner] : reg.view<ParticleSpriteOwner>().each() )
  {
    if ( owner.sprite->get_tag() == tag ) particle_sprites.emplace_back( *owner.sprite );
  }
  return particle_sprites;
}

} // namespace ProceduralMaze::Sys