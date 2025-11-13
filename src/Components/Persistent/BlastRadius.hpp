#ifndef __CMP_BLASTRADIUS_HPP__
#define __CMP_BLASTRADIUS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class BlastRadius : public BasePersistent<int>
{
public:
  BlastRadius( int value = 1, int min_value = 1, int max_value = 5 )
      : BasePersistent<int>( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "BlastRadius"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BLASTRADIUS_HPP__
