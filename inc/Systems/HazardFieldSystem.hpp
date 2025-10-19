#ifndef __SYS_HAZARDFIELDSYSTEM_HPP__
#define __SYS_HAZARDFIELDSYSTEM_HPP__

#include <BaseSystem.hpp>
#include <CorruptionCell.hpp>
#include <Door.hpp>
#include <Exit.hpp>
#include <NPC.hpp>
#include <Obstacle.hpp>
#include <Persistent/CorruptionSeed.hpp>
#include <Persistent/SinkholeSeed.hpp>
#include <PlayableCharacter.hpp>
#include <ReservedPosition.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SinkholeCell.hpp>
#include <Wall.hpp>
#include <entt/entity/fwd.hpp>
namespace ProceduralMaze::Sys {

/**
 * @brief A templated system that manages the creation and spread of hazard fields in a procedural maze.
 *
 * This system handles the lifecycle of hazard field entities, from initial seeding to progressive
 * spreading throughout the maze. It operates on entities with obstacle components and converts them
 * to hazard field entities over time.
 *
 * @tparam HazardType The type of hazard component (e.g., Cmp::SinkholeCell, Cmp::CorruptionCell)
 *                    that this system will manage. Different hazard types are prevented from
 *                    overlapping through exclusion mechanisms.
 *
 * Key behaviors:
 * - Seeds an initial hazard field at a random obstacle location during startup
 * - Periodically spreads hazard fields to adjacent obstacles with configurable probability
 * - Prevents multiple hazard types from occupying the same location
 * - Deactivates hazard fields when they become surrounded by other hazard fields
 * - Converts obstacle entities to hazard field entities, removing their obstacle component
 *
 * The system uses a time-based update mechanism with a 5-second interval between spread attempts.
 * Each active hazard field has a 1 in 8 chance of spreading to an adjacent obstacle per update cycle.
 *
 * @note Inherits from Sys::BaseSystem and requires a shared EnTT registry for entity management.
 */
template <typename HazardType> class HazardFieldSystem : public Sys::BaseSystem
{
public:
  HazardFieldSystem( ProceduralMaze::SharedEnttRegistry reg )
      : Sys::BaseSystem( reg )
  {
    init_context();
  }

  void init_context()
  {
    // ensure we have a persistent component for the hazard seed
    if constexpr ( std::is_same_v<HazardType, Cmp::SinkholeCell> )
    {
      add_persistent_component<Cmp::Persistent::SinkholeSeed>();
    }
    else if constexpr ( std::is_same_v<HazardType, Cmp::CorruptionCell> )
    {
      add_persistent_component<Cmp::Persistent::CorruptionSeed>();
    }
  }

  // Starts the hazard field process, gets view of all obstacles, adds one hazard field component at one of the
  // positions. This is done only once at the start of the game so it will check if there is already a hazard field
  // component present.
  void init_hazard_field()
  {
    // We want only one hazard type per map but we also want to avoid initializing hazards on top of other existing
    // hazards, so we exclude other hazard  types from the view
    auto hazard_field_view = [this]() {
      if constexpr ( std::is_same_v<HazardType, Cmp::SinkholeCell> )
      {
        return m_reg->view<HazardType>( entt::exclude<Cmp::CorruptionCell> );
      }
      else if constexpr ( std::is_same_v<HazardType, Cmp::CorruptionCell> )
      {
        return m_reg->view<HazardType>( entt::exclude<Cmp::SinkholeCell> );
      }
      else { return m_reg->view<HazardType>(); }
    }();

    if ( std::distance( hazard_field_view.begin(), hazard_field_view.end() ) > 0 )
    {
      SPDLOG_DEBUG( "Hazard field already seeded." );
      return;
    }

    // now find an obstacle/position to spawn the initial hazard field
    // explicitly exclude walls, doors, exits, and playable characters
    unsigned long seed = 0;
    entt::entity random_entity = entt::null;
    Cmp::Position random_position( { 0.f, 0.f } );
    if constexpr ( std::is_same_v<HazardType, Cmp::CorruptionCell> )
    {
      seed = get_persistent_component<Cmp::Persistent::CorruptionSeed>()();
      std::tie( random_entity, random_position ) = get_random_position(
          IncludePack<Cmp::Obstacle>{},
          ExcludePack<Cmp::Wall, Cmp::Door, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC, Cmp::ReservedPosition>(),
          seed );
    }
    else if constexpr ( std::is_same_v<HazardType, Cmp::SinkholeCell> )
    {
      seed = get_persistent_component<Cmp::Persistent::SinkholeSeed>()();
      std::tie( random_entity, random_position ) = get_random_position(
          IncludePack<Cmp::Obstacle>{},
          ExcludePack<Cmp::Wall, Cmp::Door, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC, Cmp::ReservedPosition>(),
          seed );
    }
    if ( random_entity == entt::null )
    {
      SPDLOG_WARN( "No valid entity found to seed hazard field." );
      return;
    }
    m_reg->emplace<HazardType>( random_entity );
    m_reg->remove<Cmp::Obstacle>( random_entity );
    SPDLOG_DEBUG( "Hazard field seeded at random obstacle index {} (entity {}).", random_index,
                  static_cast<uint32_t>( random_entity ) );
  }

  // add an adjacent hazard field component if possible
  // This is done every m_interval seconds, using the m_clock to track time.
  // get a view of all hazard fields, for each hazard field check adjacent positions
  // if there is no hazard field, add a hazard field component
  void update_hazard_field()
  {
    if ( m_clock.getElapsedTime() < m_interval_sec ) return;
    m_clock.restart();

    auto hazard_field_view = m_reg->view<HazardType, Cmp::Position>();
    auto hazard_field_hitbox_size = sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions };
    auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::ReservedPosition> );
    auto obstacle_hitbox_size = sf::Vector2f{ Sprites::MultiSprite::kDefaultSpriteDimensions };
    Cmp::Random hazard_field_spread_picker = Cmp::Random( 0, 7 ); // 1 in 8 chance for picking an adjacent obstacle

    for ( auto [hazard_field_entity, hazard_field_cmp, position_cmp] : hazard_field_view.each() )
    {
      if ( !hazard_field_cmp.active ) continue; // only active hazard fields can spread

      // make the hazard field hitbox slightly larger to find adjacent obstacles
      auto hazard_field_hitbox = sf::FloatRect( position_cmp, hazard_field_hitbox_size * 2.f );
      int adjacent_hazard_fields = 0;
      for ( auto [obstacle_entity, obstacle_cmp, obstacle_pos] : obstacle_view.each() )
      {
        auto obstacle_hitbox = sf::FloatRect( obstacle_pos, obstacle_hitbox_size );
        if ( hazard_field_hitbox.findIntersection( obstacle_hitbox ) )
        {
          // check if this obstacle already has a hazard field component
          if ( m_reg->try_get<HazardType>( obstacle_entity ) == nullptr )
          {

            if ( hazard_field_spread_picker.gen() == 0 )
            {
              // add hazard field component to this obstacle
              m_reg->emplace<HazardType>( obstacle_entity );
              m_reg->remove<Cmp::Obstacle>( obstacle_entity );
              SPDLOG_DEBUG( "New hazard field created at entity {}", static_cast<uint32_t>( obstacle_entity ) );
              // only add one hazard field per update
              return;
            }
          }
        }
      }

      for ( auto [adj_hazard_field_entity, adj_hazard_field_cmp, adj_position_cmp] : hazard_field_view.each() )
      {
        if ( hazard_field_entity == adj_hazard_field_entity ) continue; // skip self
        // check if hazard field is adjacent to other hazard fields
        auto adj_hazard_field_hitbox = sf::FloatRect( adj_position_cmp, hazard_field_hitbox_size );
        if ( hazard_field_hitbox.findIntersection( adj_hazard_field_hitbox ) ) { adjacent_hazard_fields++; }
      }
      // if the hazard field is surrounded by hazard fields, then we can exclude it from future searches
      if ( adjacent_hazard_fields >= 2 )
      {
        hazard_field_cmp.active = false;

        SPDLOG_DEBUG( "Hazard field at entity {} is now inactive (surrounded).",
                      static_cast<uint32_t>( hazard_field_entity ) );
      }
    }
  }

private:
  sf::Clock m_clock;
  const sf::Time m_interval_sec{ sf::seconds( 5.0f ) }; // seconds between adding new hazard fields
};

using SinkHoleHazardSystem = HazardFieldSystem<Cmp::SinkholeCell>;
using CorruptionHazardSystem = HazardFieldSystem<Cmp::CorruptionCell>;

} // namespace ProceduralMaze::Sys

#endif // __SYS_HAZARDFIELDSYSTEM_HPP__