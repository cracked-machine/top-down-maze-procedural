#ifndef SRC_COMPONENTS_PERSISTENT_PLANTBURNDURATION_HPP__
#define SRC_COMPONENTS_PERSISTENT_PLANTBURNDURATION_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Duration, in seconds, that a plant remains on fire before burning out.
class PlantBurnDuration : public BasePersistent<float>
{
public:
  //! @brief Construct with an initial burn duration and optional min/max bounds.
  //! @param value initial burn duration, in seconds
  //! @param min_value minimum allowed burn duration
  //! @param max_value maximum allowed burn duration
  PlantBurnDuration( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "PlantBurnDuration"
  virtual std::string class_name() const override { return "PlantBurnDuration"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist
#endif // SRC_COMPONENTS_PERSISTENT_PLANTBURNDURATION_HPP__
