#ifndef SRC_COMPONENTS_PERSISTENT_POSTPULLMOVEMENTDELAY_HPP__
#define SRC_COMPONENTS_PERSISTENT_POSTPULLMOVEMENTDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

class PostPullMovementDelay : public BasePersistent<float>
{
public:
  PostPullMovementDelay( float value = 0, float min_value = 0, float max_value = 0 )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }

  [[nodiscard]] std::string class_name() const override { return "PostPullMovementDelay"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_POSTPULLMOVEMENTDELAY_HPP__
