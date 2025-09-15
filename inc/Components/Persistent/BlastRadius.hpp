#ifndef __CMP_BLASTRADIUS_HPP__
#define __CMP_BLASTRADIUS_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class BlastRadius : public BasePersistent<int>
{
public:
  BlastRadius()
      : BasePersistent<int>( 1 )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BLASTRADIUS_HPP__
