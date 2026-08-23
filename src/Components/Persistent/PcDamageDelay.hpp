#ifndef SRC_COMPONENTS_PERSISTENT_PCDAMAGEDELAY_HPP__
#define SRC_COMPONENTS_PERSISTENT_PCDAMAGEDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace Game::Cmp::Persist
{

//! @brief Persistent setting for the delay (in seconds) before the player character can take damage
//! again after being hit.
class PcDamageDelay : public BasePersistent<float>
{
public:
  //! @brief Construct a new PcDamageDelay object.
  //! @param value Initial delay value.
  //! @param min_value Minimum allowed delay value.
  //! @param max_value Maximum allowed delay value.
  PcDamageDelay( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  //! @brief Returns the class name used for identification and JSON serialization.
  //! @return std::string The class name.
  virtual std::string class_name() const override { return "PcDamageDelay"; }
  //! @brief Returns the display detail string for this setting (currently unused/empty).
  //! @return std::string The detail string.
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_PCDAMAGEDELAY_HPP__
