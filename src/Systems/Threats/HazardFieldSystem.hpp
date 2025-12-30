#ifndef SRC_SYSTEMS_HAZARDFIELDSYSTEM_HPP__
#define SRC_SYSTEMS_HAZARDFIELDSYSTEM_HPP__

#include <Utils/Random.hpp>
#include <entt/entity/fwd.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

#include <Components/CorruptionCell.hpp>
#include <Components/HazardFieldCell.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/CorruptionSeed.hpp>
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/PlayerMortality.hpp>
#include <Components/SinkholeCell.hpp>
#include <Systems/BaseSystem.hpp>

//! @brief HazardFields are environmental dangers that spread throughout the maze,
//! posing threats to both the player and NPCs. Examples include sinkholes (composed of many
//! Cmp::SinkholeCell) that instantly kill any entity that falls into them, and corruption fields
//! (composed of many Cmp::CorruptionCell) that gradually drain health over time.

namespace ProceduralMaze::Sys
{

template <typename T>
struct HazardTraits;

// Concept to constrain valid hazard field types
template <typename T>
concept ValidHazard = requires {
  typename HazardTraits<T>::SeedType;
  typename HazardTraits<T>::ExcludeHazard;
  { HazardTraits<T>::kills_instantly } -> std::convertible_to<bool>;
  { HazardTraits<T>::mortality_state } -> std::convertible_to<Cmp::PlayerMortality::State>;
  { HazardTraits<T>::sprite_type } -> std::convertible_to<Sprites::SpriteMetaType>;
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
  HazardFieldSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  void update();
  void init_hazard_field();
  void onPause() override;
  void onResume() override;

private:
  //! @brief Update the hazard field by spreading it to adjacent positions.
  //! 1. Check if the update interval has elapsed using m_clock.
  //! 2. Get a view of all entities with the hazard field component and position.
  //! 3. For each hazard field entity:
  //!   a. Check adjacent positions for obstacles without hazard components.
  //!   b. Use a random chance (1 in 8) to convert an adjacent obstacle into a new hazard cell.
  //!   c. Count adjacent hazard fields; if surrounded by 2 or more, mark as inactive to stop
  //!   further spreading.
  void update_hazard_field();
  //! @brief Check for player collision with hazard fields.
  //! If a collision is detected, apply damage or instant death based on hazard type.
  void check_player_hazard_field_collision();
  //! @brief Check for NPC collision with hazard fields.
  //! If a collision is detected, trigger NPC death event.
  void check_npc_hazard_field_collision();

  //! @brief Clock used to track time for hazard field updates.
  //!
  sf::Clock m_spread_update_clock;

  //! @brief Time period for updating hazard fields.
  //!
  const sf::Time m_update_period{ sf::seconds( 5.0f ) }; // seconds between adding new hazard fields
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_HAZARDFIELDSYSTEM_HPP__