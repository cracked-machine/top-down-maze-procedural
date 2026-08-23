#ifndef SRC_COMPONENTS_PERSISTENT_POSTPULLMOVEMENTDELAY_HPP__
#define SRC_COMPONENTS_PERSISTENT_POSTPULLMOVEMENTDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

//! @brief Persistent setting for the delay (in seconds) before the player can move again after
//! being pulled (e.g. by a grapple/pull mechanic).
class PostPullMovementDelay : public BasePersistent<float>
{
public:
  //! @brief Construct a new PostPullMovementDelay object.
  //! @param value Initial delay value.
  //! @param min_value Minimum allowed delay value.
  //! @param max_value Maximum allowed delay value.
  PostPullMovementDelay( float value = 0, float min_value = 0, float max_value = 0 )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }

  //! @brief Returns the class name used for identification and JSON serialization.
  //! @return std::string The class name.
  [[nodiscard]] std::string class_name() const override { return "PostPullMovementDelay"; }
  //! @brief Returns the display detail string for this setting (currently unused/empty).
  //! @return std::string The detail string.
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_POSTPULLMOVEMENTDELAY_HPP__
