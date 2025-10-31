#ifndef __CMP_HEALTHBONUS_HPP__
#define __CMP_HEALTHBONUS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class HealthBonus : public BasePersistent<int>
{
public:
  HealthBonus( int value = 25 )
      : BasePersistent<int>( value )
  {
  }
  virtual std::string class_name() const override { return "HealthBonus"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_HEALTHBONUS_HPP__
