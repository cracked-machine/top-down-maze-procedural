#ifndef SRC_SYSTEMS_HAZARDFIELDSYSTEMIMPL_HPP__
#define SRC_SYSTEMS_HAZARDFIELDSYSTEMIMPL_HPP__

#include <Audio/SoundBank.hpp>
#include <Components/Exit.hpp>
#include <Components/Npc/Npc.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerDistance.hpp>
#include <Components/Player/PlayerHealth.hpp>
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
#include <Utils/Utils.hpp>

#include <Systems/Threats/HazardTraits.hpp>

namespace ProceduralMaze::Sys
{

template <ValidHazard HazardType>
HazardFieldSystem<HazardType>::HazardFieldSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                                  Audio::SoundBank &sound_bank )
    : Sys::BaseSystem( reg, window, sprite_factory, sound_bank )
{
  get_systems_event_queue().sink<Events::PauseClocksEvent>().connect<&Sys::HazardFieldSystem<HazardType>::onPause>( this );
  get_systems_event_queue().sink<Events::ResumeClocksEvent>().connect<&Sys::HazardFieldSystem<HazardType>::onResume>( this );
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::update()
{
  update_hazard_field();
  check_npc_hazard_field_collision();

  for ( auto [_ent, _sys] : getReg().template view<Cmp::System>().each() )
  {
    if ( _sys.collisions_enabled ) { check_player_hazard_field_collision(); }
  }
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::init_hazard_field()
{
  auto hazard_field_view = getReg().template view<HazardType>( entt::exclude<typename Traits::ExcludeHazard> );
  if ( hazard_field_view.size_hint() > 0 ) { return; }

  unsigned long seed = Sys::PersistSystem::get<typename Traits::SeedType>( getReg() ).get_value();
  auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
      getReg(), Utils::Rnd::IncludePack<Cmp::Obstacle>{},
      Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::Exit, Cmp::PlayerCharacter, Cmp::NPC, Cmp::ReservedPosition>(), seed );
  if ( random_entity == entt::null ) { return; }

  Factory::destroyObstacle( getReg(), random_entity );
  getReg().template emplace<HazardType>( random_entity );
  getReg().template emplace_or_replace<Cmp::SpriteAnimation>( random_entity, 0, 0, true, Traits::sprite_type, 0 );
  getReg().template emplace_or_replace<Cmp::ZOrderValue>( random_entity, random_pos.position.y - 1.f );
  getReg().template emplace_or_replace<Cmp::NpcNoPathFinding>( random_entity );
  SPDLOG_INFO( "{} hazard spawned at position [{}, {}].", Traits::sprite_type, random_pos.position.x, random_pos.position.y );
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::onPause()
{
  m_spread_update_clock.stop();
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::onResume()
{
  m_spread_update_clock.start();
}

template <ValidHazard HazardType>
void HazardFieldSystem<HazardType>::update_hazard_field()
{
  if ( m_spread_update_clock.getElapsedTime() < m_update_period ) return;
  m_spread_update_clock.restart();

  auto hazard_view = getReg().template view<HazardType, Cmp::Position>();
  auto obstacle_view = getReg().template view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::ReservedPosition> );

  Cmp::RandomInt hazard_spread_picker( 0, 7 ); // 1 in 8 chance for picking an adjacent obstacle

  for ( auto [hazard_entity, hazard_cmp, position_cmp] : hazard_view.each() )
  {
    if ( not hazard_cmp.active ) continue;

    // make the hazard field hitbox slightly larger to find adjacent obstacles
    auto hazard_hitbox = sf::FloatRect( position_cmp.position, Constants::kGridSquareSizePixelsF * 2.f );
    int adjacent_hazard_fields = 0;

    // add new hazard cell
    for ( auto [obstacle_entity, obstacle_cmp, obst_pos_cmp] : obstacle_view.each() )
    {
      if ( not hazard_hitbox.findIntersection( obst_pos_cmp ) ) continue;
      if ( getReg().template try_get<HazardType>( obstacle_entity ) ) continue;
      if ( hazard_spread_picker.gen() == 0 )
      {
        Factory::destroyObstacle( getReg(), obstacle_entity );
        getReg().template emplace_or_replace<HazardType>( obstacle_entity );
        getReg().template emplace_or_replace<Cmp::SpriteAnimation>( obstacle_entity, 0, 0, true, Traits::sprite_type, 0 );
        getReg().template emplace_or_replace<Cmp::ZOrderValue>( obstacle_entity, obst_pos_cmp.position.y - 1.f );
        getReg().template emplace_or_replace<Cmp::NpcNoPathFinding>( obstacle_entity );
        if ( getReg().template all_of<Cmp::PlayerDistance>( obstacle_entity ) ) { getReg().template remove<Cmp::PlayerDistance>( obstacle_entity ); }

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
  auto hazard_view = getReg().template view<HazardType, Cmp::Position>();
  auto player_view = getReg().template view<Cmp::PlayerCharacter, Cmp::PlayerHealth, Cmp::PlayerMortality, Cmp::Position>();
  const auto &player_position = Utils::get_player_position( getReg() );

  for ( auto [pc_entt, player_cmp, player_health_cmp, player_mort_cmp, player_pos_cmp] : player_view.each() )
  {
    // optimization
    // if ( player_mort_cmp.state != Cmp::PlayerMortality::State::ALIVE ) return;
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), player_pos_cmp ) ) continue;

    // dont spam death events if the player is already dead
    if ( player_mort_cmp.state == Cmp::PlayerMortality::State::DEAD ) continue;

    // reduce the player hitbox so that you have to be almost centered over it to fall in
    auto player_hitbox_redux = Cmp::RectBounds( player_pos_cmp.position, player_pos_cmp.size, 0.1f );
    for ( auto [hazard_entt, hazard_cmp, hazard_pos_cmp] : hazard_view.each() )
    {
      // reduce the hazaard hotbox so that you have to be almost centered over it to fall in
      auto hazard_hitbox_redux = Cmp::RectBounds( hazard_pos_cmp.position, hazard_pos_cmp.size, 0.1f );
      if ( not player_hitbox_redux.findIntersection( hazard_hitbox_redux.getBounds() ) ) continue;

      if constexpr ( Traits::sprite_type == "SINKHOLE" )
      {
        // trigger death animation
        get_systems_event_queue().trigger( Events::PlayerMortalityEvent( Cmp::PlayerMortality::State::FALLING, player_position ) );
        return;
      }
      else if constexpr ( Traits::sprite_type == "CORRUPTION" )
      {
        player_health_cmp.health -= Sys::PersistSystem::get<Cmp::Persist::CorruptionDamage>( getReg() ).get_value();
        // trigger death animation
        if ( player_health_cmp.health <= 0 )
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
  auto hazard_view = getReg().template view<HazardType, Cmp::Position>();
  auto npc_view = getReg().template view<Cmp::NPC, Cmp::Position>();

  for ( auto [npc_entt, npc_cmp, npc_pos_cmp] : npc_view.each() )
  {
    // optimization
    if ( !Utils::is_visible_in_view( RenderSystem::getGameView(), npc_pos_cmp ) ) continue;

    for ( auto [hazard_entt, hazard_cmp, hazard_pos_cmp] : hazard_view.each() )
    {
      if ( not npc_pos_cmp.findIntersection( hazard_pos_cmp ) ) continue;

      auto loot_entity = Factory::destroyNPC( getReg(), npc_entt );
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