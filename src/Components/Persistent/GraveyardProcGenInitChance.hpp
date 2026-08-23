#ifndef SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENINITCHANCE_HPP__
#define SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENINITCHANCE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Initial chance of seeding an obstacle cell before the graveyard exterior's cellular-automaton
//! procedural generation pass runs (see Sys::ProcGen::LevelGenerator::add_graveyard_exterior_obstacles).
class GraveyardProcGenInitChance : public BasePersistent<float>
{
public:
  //! @brief Construct with an initial seed chance and optional min/max bounds.
  //! @param value initial seed chance
  //! @param min_value minimum allowed seed chance
  //! @param max_value maximum allowed seed chance
  GraveyardProcGenInitChance( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "GraveyardProcGenInitChance"
  [[nodiscard]] std::string class_name() const override { return "GraveyardProcGenInitChance"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENINITCHANCE_HPP__
