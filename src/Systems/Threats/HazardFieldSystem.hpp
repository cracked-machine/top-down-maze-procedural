#ifndef __SYS_HAZARDFIELDSYSTEM_HPP__
#define __SYS_HAZARDFIELDSYSTEM_HPP__

#include <Components/Persistent/CorruptionDamage.hpp>
#include <entt/entity/fwd.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <Components/CorruptionCell.hpp>
#include <Components/Door.hpp>
#include <Components/Exit.hpp>
#include <Components/HazardFieldCell.hpp>
#include <Components/NPC.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/CorruptionSeed.hpp>
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SinkholeCell.hpp>
#include <Components/Wall.hpp>
#include <Events/NpcDeathEvent.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

//! @brief HazardFields are environmental dangers that spread throughout the maze,
//! posing threats to both the player and NPCs. Examples include sinkholes (composed of many Cmp::SinkholeCell)
//! that instantly kill any entity that falls into them, and corruption fields (composed of many Cmp::CorruptionCell)
//! that gradually drain health over time.

namespace ProceduralMaze::Sys {

template <typename T>
struct HazardTraits;

// Traits for Sinkhole hazard field types
template <>
struct HazardTraits<Cmp::SinkholeCell>
{
  using SeedType = Cmp::Persistent::SinkholeSeed;
  using ExcludeHazard = Cmp::CorruptionCell;
  static constexpr bool kills_instantly = true;
};

// Traits for Corruption hazard field types
template <>
struct HazardTraits<Cmp::CorruptionCell>
{
  using SeedType = Cmp::Persistent::CorruptionSeed;
  using ExcludeHazard = Cmp::SinkholeCell;
  static constexpr bool kills_instantly = false;
};

// Concept to constrain valid hazard field types
template <typename T>
concept ValidHazard = requires {
  typename HazardTraits<T>::SeedType;
  typename HazardTraits<T>::ExcludeHazard;
  { HazardTraits<T>::kills_instantly } -> std::convertible_to<bool>;
};

/**
 * @brief A templated system that manages the creation and spread of hazard
 * fields in a procedural maze.
 *
 * @tparam HazardType The type of hazard component that this system will manage.
 *         Must satisfy the ValidHazard concept.
 */
template <ValidHazard HazardType>
class HazardFieldSystem : public Sys::BaseSystem
{
  using Traits = HazardTraits<HazardType>;

public:
  //! @brief Construct a new Hazard Field System object
  //!
  //! @param reg Smart pointer to the entt registry
  //! @param window Reference to the SFML render window
  //! @param sprite_factory Reference to the sprite factory
  HazardFieldSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                     Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : Sys::BaseSystem( reg, window, sprite_factory, sound_bank )
  {
    // seed component must be created in entt registry before use.
    add_persistent_component<typename Traits::SeedType>();
  }

  //! @brief Initialise the hazard field. This is done only once at the start of the game:
  //! 1. Get view of all entities that own obstacles and position components
  //! 2. Return if there is already a hazard field component present
  //! 3. Pick a random entity from the view (using seed).
  //! 4. Add hazard cell component to the random entity.
  //! 5. Remove obstacle component from the random entity.
  void init_hazard_field()
  {
    auto hazard_field_view = m_reg->view<HazardType>( entt::exclude<typename Traits::ExcludeHazard> );
    if ( hazard_field_view.size_hint() > 0 ) { return; }

    unsigned long seed = get_persistent_component<typename Traits::SeedType>().get_value();
    auto [random_entity, random_pos] = get_random_position(
        IncludePack<Cmp::Obstacle>{},
        ExcludePack<Cmp::Wall, Cmp::Door, Cmp::Exit, Cmp::PlayableCharacter, Cmp::NPC, Cmp::ReservedPosition>(), seed );
    if ( random_entity == entt::null ) { return; }

    m_reg->emplace<HazardType>( random_entity );
    m_reg->remove<Cmp::Obstacle>( random_entity );
    SPDLOG_INFO( "Hazard field seeded at position [{}, {}].", random_pos.position.x, random_pos.position.y );
  }

  // add an adjacent hazard field component if possible
  // This is done every m_interval seconds, using the m_clock to track time.
  // get a view of all hazard fields, for each hazard field check adjacent
  // positions if there is no hazard field, add a hazard field component

  //! @brief Update the hazard field by spreading it to adjacent positions.
  //! 1. Check if the update interval has elapsed using m_clock.
  //! 2. Get a view of all entities with the hazard field component and position.
  //! 3. For each hazard field entity:
  //!   a. Check adjacent positions for obstacles without hazard components.
  //!   b. Use a random chance (1 in 8) to convert an adjacent obstacle into a new hazard cell.
  //!   c. Count adjacent hazard fields; if surrounded by 2 or more, mark as inactive to stop further spreading.
  void update_hazard_field()
  {
    if ( m_clock.getElapsedTime() < m_update_period ) return;
    m_clock.restart();

    auto hazard_view = m_reg->view<HazardType, Cmp::Position>();
    auto obstacle_view = m_reg->view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::ReservedPosition> );

    Cmp::RandomInt hazard_spread_picker( 0, 7 ); // 1 in 8 chance for picking an adjacent obstacle

    for ( auto [hazard_entity, hazard_cmp, position_cmp] : hazard_view.each() )
    {
      if ( not hazard_cmp.active ) continue;

      // make the hazard field hitbox slightly larger to find adjacent obstacles
      auto hazard_hitbox = sf::FloatRect( position_cmp.position, kGridSquareSizePixelsF * 2.f );
      int adjacent_hazard_fields = 0;

      // add new hazard cell
      for ( auto [obstacle_entity, obstacle_cmp, obst_pos_cmp] : obstacle_view.each() )
      {
        if ( not hazard_hitbox.findIntersection( obst_pos_cmp ) ) continue;
        if ( m_reg->try_get<HazardType>( obstacle_entity ) ) continue;
        if ( hazard_spread_picker.gen() == 0 )
        {
          m_reg->emplace<HazardType>( obstacle_entity );
          m_reg->remove<Cmp::Obstacle>( obstacle_entity );
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

      // if the hazard field is surrounded by hazard fields, then we can exclude it from future searches
      if ( adjacent_hazard_fields >= 2 ) { hazard_cmp.active = false; }
    }
  }

  //! @brief Check for player collision with hazard fields.
  //! If a collision is detected, apply damage or instant death based on hazard type.
  void check_player_hazard_field_collision()
  {
    auto hazard_view = m_reg->view<HazardType, Cmp::Position>();
    auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Position>();

    for ( auto [pc_entt, player_cmp, player_pos_cmp] : player_view.each() )
    {
      // optimization
      if ( !is_visible_in_view( RenderSystem::getGameView(), player_pos_cmp ) ) continue;

      // make a copy and reduce the player hitbox size to avoid unfair deaths
      auto offset = kGridSquareSizePixelsF / 4.f;
      auto player_hitbox = sf::FloatRect( player_pos_cmp.position + offset, offset * 1.5f );

      for ( auto [hazard_entt, hazard_cmp, hazard_pos_cmp] : hazard_view.each() )
      {
        if ( not player_hitbox.findIntersection( hazard_pos_cmp ) ) continue;

        if constexpr ( Traits::kills_instantly ) { player_cmp.alive = false; }
        else { player_cmp.health -= get_persistent_component<Cmp::Persistent::CorruptionDamage>().get_value(); }
        SPDLOG_DEBUG( "Player fell into a hazard field at position ({}, {})!", hazard_pos_cmp.x, hazard_pos_cmp.y );
        return;
      }
    }
  }

  //! @brief Check for NPC collision with hazard fields.
  //! If a collision is detected, trigger NPC death event.
  void check_npc_hazard_field_collision()
  {
    auto hazard_view = m_reg->view<HazardType, Cmp::Position>();
    auto npc_view = m_reg->view<Cmp::NPC, Cmp::Position>();

    for ( auto [npc_entt, npc_cmp, npc_pos_cmp] : npc_view.each() )
    {
      // optimization
      if ( !is_visible_in_view( RenderSystem::getGameView(), npc_pos_cmp ) ) continue;

      for ( auto [hazard_entt, hazard_cmp, hazard_pos_cmp] : hazard_view.each() )
      {
        if ( not npc_pos_cmp.findIntersection( hazard_pos_cmp ) ) continue;

        getEventDispatcher().trigger( Events::NpcDeathEvent( npc_entt ) );
        SPDLOG_DEBUG( "NPC fell into a hazard field at position ({}, {})!", hazard_pos_cmp.x, hazard_pos_cmp.y );
        return;
      }
    }
  }

private:
  //! @brief Clock used to track time for hazard field updates.
  //!
  sf::Clock m_clock;

  //! @brief Time period for updating hazard fields.
  //!
  const sf::Time m_update_period{ sf::seconds( 5.0f ) }; // seconds between adding new hazard fields
};

//! @brief Sinkhole hazard system.
//! Specialization of HazardFieldSystem for Cmp::SinkholeCell.
using SinkHoleHazardSystem = HazardFieldSystem<Cmp::SinkholeCell>;

//! @brief Corruption hazard system.
//! Specialization of HazardFieldSystem for Cmp::CorruptionCell.
using CorruptionHazardSystem = HazardFieldSystem<Cmp::CorruptionCell>;

} // namespace ProceduralMaze::Sys

#endif // __SYS_HAZARDFIELDSYSTEM_HPP__