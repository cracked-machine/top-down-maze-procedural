
#include <Optimizations.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Sys
{

ParticleSystem::ParticleSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

std::vector<entt::entity> ParticleSystem::add_to_registry( std::vector<ParticleSpriteOwner> owners )
{
  std::vector<entt::entity> entt_list;
  for ( auto &owner : owners )
  {
    auto entt = reg().create();
    reg().emplace<ParticleSpriteOwner>( entt, std::move( owner ) );
    entt_list.push_back( entt );
    SPDLOG_INFO( "Created ParticleSprite {}", static_cast<uint32_t>( entt ) );
  }
  return entt_list;
}

void ParticleSystem::update( sf::Time dt )
{
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
  if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_game_view(), target ) ) return;
  for ( auto [entt, owner] : reg().view<ParticleSpriteOwner>().each() )
  {
    if ( owner.sprite->is_active() )
    {
      SPDLOG_DEBUG( "Simulating" );
      owner.sprite->check_particle_collision( target );
    }
  }
}

[[nodiscard]] Cmp::Particle::IParticleSprite *ParticleSystem::find( entt::registry &reg, const std::string &tag )
{
  for ( auto [entt, owner] : reg.view<ParticleSpriteOwner>().each() )
  {
    if ( owner.sprite->get_tag() == tag ) return owner.sprite.get();
  }
  return nullptr;
}

} // namespace ProceduralMaze::Sys