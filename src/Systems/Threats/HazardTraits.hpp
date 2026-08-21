#ifndef SRC_SYSTEMS_THREATS_HAZARDTRAITS_HPP__
#define SRC_SYSTEMS_THREATS_HAZARDTRAITS_HPP__

#include <Components/Hazard/CorruptionCell.hpp>
#include <Components/Hazard/SinkholeCell.hpp>
#include <Components/Persistent/CorruptionSeed.hpp>
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/Player/Mortality.hpp>
#include <Sprites/SpriteSheet.hpp>

namespace Game::Sys
{

//! @brief Traits describing how a HazardFieldSystem should behave for a given hazard cell component type. Specialized per hazard type.
//! @tparam T
template <typename T>
struct HazardTraits;

//! @brief Traits for Sinkhole hazard field types: kills instantly on contact and excludes corruption cells.
template <>
struct HazardTraits<Cmp::SinkholeCell>
{
  using SeedType = Cmp::Persist::SinkholeSeed;
  using ExcludeHazard = Cmp::CorruptionCell;
  static constexpr bool kills_instantly = true;
  static constexpr Cmp::Player::Mortality::State mortality_state = Cmp::Player::Mortality::State::FALLING;
  static constexpr std::string_view sprite_type = "sprite.graveyard.hazard.sinkhole";
  static constexpr int odds = 2;
};

//! @brief Traits for Corruption hazard field types: gradually drains health on contact and excludes sinkhole cells.
template <>
struct HazardTraits<Cmp::CorruptionCell>
{
  using SeedType = Cmp::Persist::CorruptionSeed;
  using ExcludeHazard = Cmp::SinkholeCell;
  static constexpr bool kills_instantly = false;
  static constexpr Cmp::Player::Mortality::State mortality_state = Cmp::Player::Mortality::State::DECAYING;
  static constexpr std::string_view sprite_type = "sprite.graveyard.hazard.corruption";
  static constexpr int odds = 7;
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_THREATS_HAZARDTRAITS_HPP__
