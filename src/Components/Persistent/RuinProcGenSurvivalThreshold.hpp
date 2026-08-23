#ifndef SRC_COMPONENTS_PERSISTENT_RUINPROCGENSURVIVALTHRESHOLD_HPP__
#define SRC_COMPONENTS_PERSISTENT_RUINPROCGENSURVIVALTHRESHOLD_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Neighbour count threshold a cell must meet to survive during ruin procedural generation.
class RuinProcGenSurvivalThreshold : public BasePersistent<uint8_t>
{
public:
  //! @brief Construct with an initial threshold value and optional min/max bounds.
  //! @param value initial threshold value
  //! @param min_value minimum allowed threshold value
  //! @param max_value maximum allowed threshold value
  RuinProcGenSurvivalThreshold( uint8_t value = 0.f, uint8_t min_value = 0.f, uint8_t max_value = 0.f )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "RuinProcGenSurvivalThreshold"
  [[nodiscard]] std::string class_name() const override { return "RuinProcGenSurvivalThreshold"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_RUINPROCGENSURVIVALTHRESHOLD_HPP__
