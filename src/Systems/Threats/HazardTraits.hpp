#ifndef SRC_SYSTEMS_HAZARDTRAITS_HPP__
#define SRC_SYSTEMS_HAZARDTRAITS_HPP__

#include <Components/Hazard/CorruptionCell.hpp>
#include <Components/Hazard/SinkholeCell.hpp>
#include <Components/Persistent/CorruptionSeed.hpp>
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/Player/PlayerMortality.hpp>
#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Sys
{

template <typename T>
struct HazardTraits;

// Traits for Sinkhole hazard field types
template <>
struct HazardTraits<Cmp::SinkholeCell>
{
  using SeedType = Cmp::Persist::SinkholeSeed;
  using ExcludeHazard = Cmp::CorruptionCell;
  static constexpr bool kills_instantly = true;
  static constexpr Cmp::PlayerMortality::State mortality_state = Cmp::PlayerMortality::State::FALLING;
  static constexpr Sprites::SpriteMetaType sprite_type = "SINKHOLE";
};

// Traits for Corruption hazard field types
template <>
struct HazardTraits<Cmp::CorruptionCell>
{
  using SeedType = Cmp::Persist::CorruptionSeed;
  using ExcludeHazard = Cmp::SinkholeCell;
  static constexpr bool kills_instantly = false;
  static constexpr Cmp::PlayerMortality::State mortality_state = Cmp::PlayerMortality::State::DECAYING;
  static constexpr Sprites::SpriteMetaType sprite_type = "CORRUPTION";
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_HAZARDTRAITS_HPP__