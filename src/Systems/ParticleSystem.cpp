#include <Optimizations.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
{

ParticleSystem::ParticleSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

void ParticleSystem::add( std::vector<ParticleSpriteOwner> owners )
{
  for ( auto &owner : owners )
  {
    auto entt = getReg().create();
    getReg().emplace<ParticleSpriteOwner>( entt, std::move( owner ) );
    SPDLOG_INFO( "Created ParticleSprite {}", static_cast<uint32_t>( entt ) );
  }
}

void ParticleSystem::update( sf::Time dt )
{
  for ( auto [entt, owner] : getReg().view<ParticleSpriteOwner>().each() )
  {
    if ( owner.sprite->is_active() )
    {
      SPDLOG_DEBUG( "Simulating" );
      owner.sprite->simulate( dt );
    }
  }
}

void ParticleSystem::check_collsion( const sf::FloatRect &target )
{
  if ( not Utils::is_visible_in_view( Sys::RenderSystem::getGameView(), target ) ) return;
  for ( auto [entt, owner] : getReg().view<ParticleSpriteOwner>().each() )
  {
    if ( owner.sprite->is_active() )
    {
      SPDLOG_DEBUG( "Simulating" );
      owner.sprite->check_collision( target );
    }
  }
}

[[nodiscard]] Cmp::IParticleSprite *ParticleSystem::find( entt::registry &reg, const std::string &tag )
{
  for ( auto [entt, owner] : reg.view<ParticleSpriteOwner>().each() )
  {
    if ( owner.tag == tag ) return owner.sprite.get();
  }
  return nullptr;
}

} // namespace ProceduralMaze::Sys