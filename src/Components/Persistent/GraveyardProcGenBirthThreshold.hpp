#ifndef SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENBIRTHTHRESHOLD_HPP__
#define SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENBIRTHTHRESHOLD_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Minimum number of live neighbour cells required for a dead cell to become alive (birth) during
//! the graveyard exterior's cellular-automaton procedural generation pass (see Sys::CellAutomataSystem).
class GraveyardProcGenBirthThreshold : public BasePersistent<uint8_t>
{
public:
  //! @brief Construct with an initial birth threshold and optional min/max bounds.
  //! @param value initial birth threshold, in live neighbour count
  //! @param min_value minimum allowed birth threshold
  //! @param max_value maximum allowed birth threshold
  GraveyardProcGenBirthThreshold( uint8_t value = 0.f, uint8_t min_value = 0.f, uint8_t max_value = 0.f )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "GraveyardProcGenBirthThreshold"
  [[nodiscard]] std::string class_name() const override { return "GraveyardProcGenBirthThreshold"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENBIRTHTHRESHOLD_HPP__
