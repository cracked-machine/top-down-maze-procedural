#ifndef SRC_COMPONENTS_PERSISTENT_PLAYERDIAGONALLERPSPEEDMODIFIER_HPP__
#define SRC_COMPONENTS_PERSISTENT_PLAYERDIAGONALLERPSPEEDMODIFIER_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace Game::Cmp::Persist
{

class PlayerDiagonalLerpSpeedModifier : public BasePersistent<float>
{
public:
  PlayerDiagonalLerpSpeedModifier( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerDiagonalLerpSpeedModifier"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_PLAYERDIAGONALLERPSPEEDMODIFIER_HPP__
