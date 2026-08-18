#ifndef SRC_COMPONENTS_PERSISTENT_HAZARDPUSHBACKRESIST_HPP__
#define SRC_COMPONENTS_PERSISTENT_HAZARDPUSHBACKRESIST_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief How long (seconds) the player must keep pushing into a lethal hazard cell before
//!        Cmp::Hazard::CollisionResist yields and lets them step/fall in.
struct HazardPushbackResist : BasePersistent<float>
{
  HazardPushbackResist( float value = 0, float min_value = 0, float max_value = 0 )
      : BasePersistent( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "HazardPushbackResist"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_HAZARDPUSHBACKRESIST_HPP__
