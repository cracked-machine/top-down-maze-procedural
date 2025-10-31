#ifndef _PERSISTENT_WEAPONS_DEGRADE_PER_HIT_HPP__
#define _PERSISTENT_WEAPONS_DEGRADE_PER_HIT_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

struct WeaponDegradePerHit : public BasePersistent<float>
{
  WeaponDegradePerHit( float value = 2.f )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "WeaponDegradePerHit"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // _PERSISTENT_WEAPONS_DEGRADE_PER_HIT_HPP__