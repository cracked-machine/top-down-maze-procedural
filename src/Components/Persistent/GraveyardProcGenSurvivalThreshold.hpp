#ifndef SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENSURVIVALTHRESHOLD_HPP__
#define SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENSURVIVALTHRESHOLD_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Minimum number of live neighbour cells required for a live cell to survive during the
//! graveyard exterior's cellular-automaton procedural generation pass (see Sys::CellAutomataSystem).
class GraveyardProcGenSurvivalThreshold : public BasePersistent<uint8_t>
{
public:
  //! @brief Construct with an initial survival threshold and optional min/max bounds.
  //! @param value initial survival threshold, in live neighbour count
  //! @param min_value minimum allowed survival threshold
  //! @param max_value maximum allowed survival threshold
  GraveyardProcGenSurvivalThreshold( uint8_t value = 0.f, uint8_t min_value = 0.f, uint8_t max_value = 0.f )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "GraveyardProcGenSurvivalThreshold"
  [[nodiscard]] std::string class_name() const override { return "GraveyardProcGenSurvivalThreshold"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENSURVIVALTHRESHOLD_HPP__
