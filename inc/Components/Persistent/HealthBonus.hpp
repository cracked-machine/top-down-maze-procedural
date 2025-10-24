#ifndef __CMP_HEALTHBONUS_HPP__
#define __CMP_HEALTHBONUS_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class HealthBonus : public BasePersistent<int>
{
public:
  HealthBonus( int value )
      : BasePersistent<int>( value )
  {
  }
  virtual std::string class_name() const override { return "HealthBonus"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_HEALTHBONUS_HPP__
