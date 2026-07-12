
#include <Audio/SoundBank.hpp>
#include <Factory/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>

#include <entt/entity/registry.hpp>

namespace Game::Sys
{

BaseSystem::BaseSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : m_reg( reg ),
      m_window( window ),
      m_sprite_factory( sprite_factory ),
      m_sound_bank( sound_bank )
{
  SPDLOG_DEBUG( "BaseSystem constructor called" );
}

// initialised by first call to getEventDispatcher()
std::unique_ptr<entt::dispatcher> BaseSystem::m_systems_event_queue = nullptr;

} // namespace Game::Sys