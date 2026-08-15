#include <Components/Npc/Npc.hpp>
#include <Components/Npc/Watchman.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/NpcWatchmanSpawnCooldown.hpp>
#include <Components/Persistent/NpcWatchmanSpawnMax.hpp>
#include <Components/Player/Character.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Factory/NpcFactory.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Threats/WatchmanSystem.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>

#include <spdlog/spdlog.h>

namespace Game::Sys
{

WatchmanSystem::WatchmanSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
  SPDLOG_DEBUG( "WatchmanSystem initialized" );
  m_watchman_spawn_timer = sf::Time::Zero;
}

void WatchmanSystem::update( sf::Time dt )
{
  m_watchman_spawn_timer += dt;
  if ( m_watchman_spawn_timer.asSeconds() >= Sys::PersistSystem::get<Cmp::Persist::NpcWatchmanSpawnCooldown>( reg() ).get_value() )
  {
    spawn_watchman();
    m_watchman_spawn_timer = sf::Time::Zero;
  }
}

void WatchmanSystem::spawn_watchman()
{
  size_t watchman_npc_count = 0;
  for ( auto [npc_entt, npc_cmp] : reg().view<Cmp::Npc::NPC>().each() )
  {
    if ( reg().any_of<Cmp::Npc::Watchman>( npc_entt ) ) { watchman_npc_count++; }
  }

  if ( watchman_npc_count < Sys::PersistSystem::get<Cmp::Persist::NpcWatchmanSpawnMax>( reg() ).get_value() and
       Utils::Player::get_player_stats( reg() ).infamy() >= 10 )
  {
    auto [rnd_entt, rnd_pos_cmp] = Utils::Rnd::get_random_position(
        reg(), {}, Utils::Rnd::ExcludePack<Cmp::Player::Character, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );
    Factory::Npc::create_npc( reg(), rnd_entt, "npc.nightwatchman" );
    SPDLOG_INFO( "Spawned Watchman at {},{}", rnd_pos_cmp.x(), rnd_pos_cmp.y() );
  }
}

} // namespace Game::Sys
