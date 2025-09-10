#ifndef __CMP_WATERBONUS_HPP__
#define __CMP_WATERBONUS_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class WaterBonus : public BasePersistent<float>
{
public:
  WaterBonus() : BasePersistent<float>( 100.0f ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WATERBONUS_HPP__
