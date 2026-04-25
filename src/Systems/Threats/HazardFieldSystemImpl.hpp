#ifndef SRC_SYSTEMS_HAZARDFIELDSYSTEMIMPL_HPP__
#define SRC_SYSTEMS_HAZARDFIELDSYSTEMIMPL_HPP__

#include <Audio/SoundBank.hpp>
#include <Components/Exit.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/System.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/PauseClocksEvent.hpp>
#include <Events/PlayerMortalityEvent.hpp>
#include <Events/ResumeClocksEvent.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Threats/HazardFieldSystem.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

#include <Systems/Threats/HazardTraits.hpp>

namespace ProceduralMaze::Sys
{

template <ValidHazard HazardType>
HazardFieldSystem<HazardType>::HazardFieldSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                                  Audio::SoundBank &sound_bank )
    : Sys::BaseSystem( reg, window, sprite_factory, sound_bank )
{
  get_systems_event_queue().sink<Events::PauseClocksEvent>().connect<&Sys::HazardFieldSystem<HazardType>::on_pause>( this );
  get_systems_event_queue().sink<Events::ResumeClocksEvent>().connect<&Sys::HazardFieldSystem<HazardType>::on_resume>( this );
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::update()
{
  update_hazard_field();
  check_npc_hazard_field_collision();

  for ( auto [_ent, _sys] : reg().template view<Cmp::System>().each() )
  {
    if ( not _sys.collisions_disabled ) { check_player_hazard_field_collision(); }
  }
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::init_hazard_field()
{
  auto hazard_field_view = reg().template view<HazardType>( entt::exclude<typename Traits::ExcludeHazard> );
  if ( hazard_field_view.size_hint() > 0 ) { return; }

  unsigned long seed = Sys::PersistSystem::get<typename Traits::SeedType>( reg() ).get_value();
  auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
      reg(), Utils::Rnd::IncludePack<Cmp::Obstacle>{},
      Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::Exit, Cmp::PlayerCharacter, Cmp::NPC, Cmp::ReservedPosition>(), seed );
  if ( random_entity == entt::null ) { return; }

  Factory::remove_obstacle( reg(), random_entity );
  reg().template emplace<HazardType>( random_entity );
  reg().template emplace_or_replace<Cmp::SpriteAnimation>( random_entity, 0, 0, true, Traits::sprite_type, 0 );
  reg().template emplace_or_replace<Cmp::ZOrderValue>( random_entity, random_pos.position.y - 1.f );
  reg().template emplace_or_replace<Cmp::NpcNoPathFinding>( random_entity );
  SPDLOG_INFO( "{} hazard spawned at position [{}, {}].", Traits::sprite_type, random_pos.position.x, random_pos.position.y );
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::on_pause()
{
  m_spread_update_clock.stop();
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::on_resume()
{
  m_spread_update_clock.start();
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::update_hazard_field()
{
  if ( m_spread_update_clock.getElapsedTime() < m_update_period ) return;
  m_spread_update_clock.restart();

  auto hazard_view = reg().template view<HazardType, Cmp::Position>();
  auto obstacle_view = reg().template view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::ReservedPosition> );

  Cmp::RandomInt hazard_spread_picker( 0, 7 ); // 1 in 8 chance for picking an adjacent obstacle

  for ( auto [hazard_entity, hazard_cmp, position_cmp] : hazard_view.each() )
  {
    if ( not hazard_cmp.active ) continue;

    // make the hazard field hitbox slightly larger to find adjacent obstacles
    auto hazard_hitbox = sf::FloatRect( position_cmp.position, Constants::kGridSizePxF * 2.f );
    int adjacent_hazard_fields = 0;

    // add new hazard cell
    for ( auto [obstacle_entity, obstacle_cmp, obst_pos_cmp] : obstacle_view.each() )
    {
      if ( not hazard_hitbox.findIntersection( obst_pos_cmp ) ) continue;
      if ( reg().template try_get<HazardType>( obstacle_entity ) ) continue;
      if ( hazard_spread_picker.gen() == 0 )
      {
        Factory::remove_obstacle( reg(), obstacle_entity );
        reg().template emplace_or_replace<HazardType>( obstacle_entity );
        reg().template emplace_or_replace<Cmp::SpriteAnimation>( obstacle_entity, 0, 0, true, Traits::sprite_type, 0 );
        reg().template emplace_or_replace<Cmp::ZOrderValue>( obstacle_entity, obst_pos_cmp.position.y - 1.f );
        reg().template emplace_or_replace<Cmp::NpcNoPathFinding>( obstacle_entity );

        SPDLOG_DEBUG( "New hazard field created at entity {}", static_cast<uint32_t>( obstacle_entity ) );
        return; // only add one hazard cell per update period
      }
    }

    // update adjacent hazard cell count
    for ( auto [adj_hazard_entity, adj_hazard_cmp, adj_pos_cmp] : hazard_view.each() )
    {
      if ( hazard_entity == adj_hazard_entity ) continue;
      if ( hazard_hitbox.findIntersection( adj_pos_cmp ) ) { adjacent_hazard_fields++; }
    }

    // if the hazard field is surrounded by hazard fields, then we can exclude it from future
    // searches
    if ( adjacent_hazard_fields >= 2 ) { hazard_cmp.active = false; }
  }
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::check_player_hazard_field_collision()
{
  auto hazard_view = reg().template view<HazardType, Cmp::Position>();
  auto player_view = reg().template view<Cmp::PlayerCharacter, Cmp::PlayerStats, Cmp::PlayerMortality, Cmp::Position>();
  const auto &player_position = Utils::Player::get_position( reg() );

  for ( auto [pc_entt, player_cmp, player_stats_cmp, player_mort_cmp, player_pos_cmp] : player_view.each() )
  {
    // optimization
    // if ( player_mort_cmp.state != Cmp::PlayerMortality::State::ALIVE ) return;
    if ( !Utils::is_visible_in_view( RenderSystem::get_world_view(), player_pos_cmp ) ) continue;

    // dont spam death events if the player is already dead
    if ( player_mort_cmp.state == Cmp::PlayerMortality::State::DEAD ) continue;

    // reduce the player hitbox so that you have to be almost centered over it to fall in
    auto player_hitbox_redux = Cmp::RectBounds::scaled( player_pos_cmp.position, player_pos_cmp.size, 0.1f );
    for ( auto [hazard_entt, hazard_cmp, hazard_pos_cmp] : hazard_view.each() )
    {
      // reduce the hazaard hotbox so that you have to be almost centered over it to fall in
      auto hazard_hitbox_redux = Cmp::RectBounds::scaled( hazard_pos_cmp.position, hazard_pos_cmp.size, 0.1f );
      if ( not player_hitbox_redux.findIntersection( hazard_hitbox_redux.getBounds() ) ) continue;

      if constexpr ( Traits::sprite_type == "SINKHOLE" )
      {
        // trigger death animation
        get_systems_event_queue().trigger( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::FALLING, player_position ) );
        return;
      }
      else if constexpr ( Traits::sprite_type == "CORRUPTION" )
      {
        auto corruption_dmg = Sys::PersistSystem::get<Cmp::Persist::CorruptionDamage>( reg() ).get_value();
        player_stats_cmp.apply_modifiers( { Cmp::Stats::Health{ -corruption_dmg }, {}, {}, {} } );

        // trigger death animation
        if ( player_stats_cmp.health() <= 0 )
        {
          get_systems_event_queue().trigger( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::DECAYING, player_position ) );
        }
        return;
      }
    }
  }
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::check_npc_hazard_field_collision()
{
  auto hazard_view = reg().template view<HazardType, Cmp::Position>();
  auto npc_view = reg().template view<Cmp::NPC, Cmp::Position>();

  for ( auto [npc_entt, npc_cmp, npc_pos_cmp] : npc_view.each() )
  {
    // optimization
    if ( !Utils::is_visible_in_view( RenderSystem::get_world_view(), npc_pos_cmp ) ) continue;

    for ( auto [hazard_entt, hazard_cmp, hazard_pos_cmp] : hazard_view.each() )
    {
      if ( not npc_pos_cmp.findIntersection( hazard_pos_cmp ) ) continue;

      auto loot_entity = Factory::destroy_npc( reg(), npc_entt );
      if ( loot_entity != entt::null )
      {
        SPDLOG_INFO( "Dropped RELIC_DROP loot at NPC death position." );
        m_sound_bank.get_effect( "drop_relic" ).play();
      }
      SPDLOG_DEBUG( "NPC fell into a hazard field at position ({}, {})!", hazard_pos_cmp.x, hazard_pos_cmp.y );
      return;
    }
  }
}

//! @brief Concrete derived HazardFieldSystem using CRTP
class SinkHoleHazardSystem final : public HazardFieldSystem<Cmp::SinkholeCell>
{
public:
  using HazardFieldSystem<Cmp::SinkholeCell>::HazardFieldSystem;
};

//! @brief Concrete derived HazardFieldSystem using CRTP
class CorruptionHazardSystem final : public HazardFieldSystem<Cmp::CorruptionCell>
{
public:
  using HazardFieldSystem<Cmp::CorruptionCell>::HazardFieldSystem;

  // Add corruption-specific methods here if needed
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_HAZARDFIELDSYSTEMIMPL_HPP__