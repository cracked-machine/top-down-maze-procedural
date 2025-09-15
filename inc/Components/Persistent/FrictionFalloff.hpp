#ifndef __CMP_FRICTIONFALLOFF_HPP__
#define __CMP_FRICTIONFALLOFF_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class FrictionFalloff : public BasePersistent<float>
{
public:
  FrictionFalloff()
      : BasePersistent<float>( 0.5f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_FRICTIONFALLOFF_HPP__