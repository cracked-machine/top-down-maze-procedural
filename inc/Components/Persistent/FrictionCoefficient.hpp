#ifndef __CMP_FRICTIONCOEFFICIENT_HPP__
#define __CMP_FRICTIONCOEFFICIENT_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class FrictionCoefficient : public BasePersistent<float>
{
public:
  FrictionCoefficient()
      : BasePersistent<float>( 0.02f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_FRICTIONCOEFFICIENT_HPP__