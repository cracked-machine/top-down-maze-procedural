#ifndef _PERSISTENT_WEAPONS_DEGRADE_PER_HIT_HPP__
#define _PERSISTENT_WEAPONS_DEGRADE_PER_HIT_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

struct WeaponDegradePerHit : public BasePersistent<float>
{
  WeaponDegradePerHit( float value = 2.f, float min_value = 0.01f, float max_value = 5.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "WeaponDegradePerHit"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // _PERSISTENT_WEAPONS_DEGRADE_PER_HIT_HPP__