#ifndef SRC_COMPONENTS_PERSISTENT_RUINPROCGENMAXITERATIONS_HPP__
#define SRC_COMPONENTS_PERSISTENT_RUINPROCGENMAXITERATIONS_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Maximum number of cellular automaton iterations to run when procedurally generating a ruin.
class RuinProcGenMaxIterations : public BasePersistent<uint16_t>
{
public:
  //! @brief Construct with an initial iteration count and optional min/max bounds.
  //! @param value initial iteration count
  //! @param min_value minimum allowed iteration count
  //! @param max_value maximum allowed iteration count
  RuinProcGenMaxIterations( uint16_t value = 0.f, uint16_t min_value = 0.f, uint16_t max_value = 0.f )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }
  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "RuinProcGenMaxIterations"
  [[nodiscard]] std::string class_name() const override { return "RuinProcGenMaxIterations"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_RUINPROCGENMAXITERATIONS_HPP__
