#ifndef SRC_COMPONENTS_PERSISTENT_RUINPROCGENINITCHANCE_HPP__
#define SRC_COMPONENTS_PERSISTENT_RUINPROCGENINITCHANCE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Initial fill probability used to seed the ruin procedural generation cellular automaton.
class RuinProcGenInitChance : public BasePersistent<float>
{
public:
  //! @brief Construct with an initial chance value and optional min/max bounds.
  //! @param value initial chance value
  //! @param min_value minimum allowed chance value
  //! @param max_value maximum allowed chance value
  RuinProcGenInitChance( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "RuinProcGenInitChance"
  [[nodiscard]] std::string class_name() const override { return "RuinProcGenInitChance"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_RUINPROCGENINITCHANCE_HPP__
