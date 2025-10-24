#ifndef __CMP_WATERBONUS_HPP__
#define __CMP_WATERBONUS_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class WaterBonus : public BasePersistent<float>
{
public:
  WaterBonus( float value )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "WaterBonus"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WATERBONUS_HPP__
