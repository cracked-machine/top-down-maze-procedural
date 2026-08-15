#include <Audio/SoundBank.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/Skeleton.hpp>
#include <Components/Npc/Watchman.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/NpcWatchmanGunFireRate.hpp>
#include <Components/Persistent/NpcWatchmanSpawnCooldown.hpp>
#include <Components/Persistent/NpcWatchmanSpawnInfamy.hpp>
#include <Components/Persistent/NpcWatchmanSpawnMax.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/Player/Character.hpp>
#include <Components/Player/Mortality.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SceneSettings/CollisionDetection.hpp>
#include <Components/Stats/ProjectileAction.hpp>
#include <Components/UUID.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ParticleFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SFML/Audio/Sound.hpp>
#include <Systems/ParticleSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Stores/NpcStore.hpp>
#include <Systems/Threats/WatchmanSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

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

  if ( Utils::Player::get_mortality( reg() ).state == Cmp::Player::Mortality::State::ALIVE )
  {
    const auto gun_fire_rate = Sys::PersistSystem::get<Cmp::Persist::NpcWatchmanGunFireRate>( reg() ).get_value();
    m_watchman_gunfire_timer += dt;
    if ( m_gun_needs_reloading and m_watchman_gunfire_timer.asSeconds() >= gun_fire_rate / 2 and
         m_watchman_gunfire_timer.asSeconds() < gun_fire_rate )
    {
      cock_gun();
      m_gun_needs_reloading = false;
    }
    else if ( m_watchman_gunfire_timer.asSeconds() >= gun_fire_rate )
    {
      fire_gun();
      m_gun_needs_reloading = true;
    }
  }

  // update the gunfire position so that it follows NPC
  for ( auto [npc_entt, npc_cmp, pos_cmp, uuid_cmp] : reg().view<Cmp::Npc::Watchman, Cmp::Position, Cmp::UUID>().each() )
  {
    Factory::Particle::update_position( reg(), uuid_cmp, pos_cmp.getCenter() );
  }

  check_gunfire_player_collision();
  check_gunfire_npc_collision();
}

void WatchmanSystem::spawn_watchman()
{
  size_t watchman_npc_count = 0;
  for ( auto [npc_entt, npc_cmp] : reg().view<Cmp::Npc::NPC>().each() )
  {
    if ( reg().any_of<Cmp::Npc::Watchman>( npc_entt ) ) { watchman_npc_count++; }
  }

  if ( watchman_npc_count < Sys::PersistSystem::get<Cmp::Persist::NpcWatchmanSpawnMax>( reg() ).get_value() and
       Utils::Player::get_player_stats( reg() ).infamy() >= Sys::PersistSystem::get<Cmp::Persist::NpcWatchmanSpawnInfamy>( reg() ).get_value() )
  {
    auto [rnd_entt, rnd_pos_cmp] = Utils::Rnd::get_random_position(
        reg(), {}, Utils::Rnd::ExcludePack<Cmp::Player::Character, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );
    Factory::Npc::create_npc( reg(), rnd_entt, "npc.nightwatchman" );
    SPDLOG_INFO( "Spawned Watchman at {},{}", rnd_pos_cmp.x(), rnd_pos_cmp.y() );
  }
}

void WatchmanSystem::fire_gun()
{
  for ( auto [npc_entt, npc_cmp, npc_pos_cmp, npc_uuid_cmp] : reg().view<Cmp::Npc::Watchman, Cmp::Position, Cmp::UUID>().each() )
  {
    if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), npc_pos_cmp ) ) continue;
    Factory::Particle::add_watchman_gunfire_ps( m_reg, "watchman.gunfire.particles", 5.0, 100.f, npc_uuid_cmp, npc_pos_cmp.getCenter(), 50000 );
    if ( m_sound_bank.get_effect( "shotgun_fire" ).getStatus() != sf::Sound::Status::Playing ) m_sound_bank.get_effect( "shotgun_fire" ).play();
  }
  m_watchman_gunfire_timer = sf::Time::Zero;
}

void WatchmanSystem::cock_gun()
{
  for ( auto [npc_entt, npc_cmp, npc_pos_cmp, npc_uuid_cmp] : reg().view<Cmp::Npc::Watchman, Cmp::Position, Cmp::UUID>().each() )
  {
    if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), npc_pos_cmp ) ) continue;
    if ( m_sound_bank.get_effect( "shotgun_rack" ).getStatus() != sf::Sound::Status::Playing ) m_sound_bank.get_effect( "shotgun_rack" ).play();
  }
}

void WatchmanSystem::check_gunfire_player_collision()
{
  if ( not Utils::scene_setting<Cmp::SceneSettings::CollisionDetection>( reg() ).enabled ) return;

  auto watchman_npc_cmp = Sys::NpcStore::instance().get_item( "npc.nightwatchman" );
  auto gunfire_projectile_action = watchman_npc_cmp.actions.at( std::type_index( typeid( Cmp::ProjectileAction ) ) );
  auto &pc_damage_cooldown = Sys::PersistSystem::get<Cmp::Persist::PcDamageDelay>( reg() );

  auto player_view = reg().view<Cmp::Player::Character, Cmp::Position, Cmp::PlayerStats, Cmp::Player::Mortality>();
  for ( auto [player_entt, player_cmp, player_pos_cmp, player_stats_cmp, player_mort_cmp] : player_view.each() )
  {
    // don't spam death events if the player is already dead
    if ( player_mort_cmp.state == Cmp::Player::Mortality::State::DEAD ) continue;
    if ( not player_cmp.skip_damage_cooldown_once &&
         player_cmp.m_damage_cooldown_timer.getElapsedTime().asSeconds() < pc_damage_cooldown.get_value() )
      continue;

    bool hit = false;
    for ( auto &gunfire_sprite : Sys::ParticleSystem::find( reg(), "watchman.gunfire.particles" ) )
    {
      if ( gunfire_sprite.get().check_particle_collision( player_pos_cmp ) ) hit = true;
    }
    if ( not hit ) continue;

    player_stats_cmp.apply_modifiers( gunfire_projectile_action.action );
    player_cmp.skip_damage_cooldown_once = false;
    m_sound_bank.get_effect( "damage_player" ).play();
    player_cmp.m_damage_cooldown_timer.restart();
    SPDLOG_INFO( "Player (health:{}) hit by Watchman gunfire", player_stats_cmp.health() );

    if ( player_stats_cmp.health() <= 0 )
    {
      if ( Utils::Player::player_has_extra_life( reg() ) )
      {
        Utils::Player::get_position( reg() ).position = Sys::PersistSystem::get<Cmp::Persist::PlayerStartPosition>( reg() );
        Factory::Player::remove_player_extra_life( reg() );
        m_sound_bank.get_effect( "player_respawn" ).play();
      }
      else { get_systems_event_queue().enqueue( Events::PlayerMortalityEvent( Cmp::Player::Mortality::State::HAUNTED, player_pos_cmp ) ); }
    }
  }
}

void WatchmanSystem::check_gunfire_npc_collision()
{
  if ( not Utils::scene_setting<Cmp::SceneSettings::CollisionDetection>( reg() ).enabled ) return;

  for ( auto [npc_entt, npc_cmp, skeleton_cmp, npc_pos_cmp] : reg().view<Cmp::Npc::NPC, Cmp::Npc::Skeleton, Cmp::Position>().each() )
  {
    bool hit = false;
    for ( auto &gunfire_sprite : Sys::ParticleSystem::find( reg(), "watchman.gunfire.particles" ) )
    {
      if ( gunfire_sprite.get().check_particle_collision( npc_pos_cmp ) ) hit = true;
    }
    if ( not hit ) continue;

    SPDLOG_INFO( "Skeleton NPC entity {} killed by Watchman gunfire", static_cast<int>( npc_entt ) );
    Factory::Npc::create_npc_death_anim( reg(), npc_pos_cmp, "sprite.death.anim.bloodsplat" );
    Factory::Npc::destroy_npc( reg(), npc_entt );
  }
}

} // namespace Game::Sys
