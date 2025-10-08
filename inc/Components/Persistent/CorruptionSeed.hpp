#ifndef __CMP_CORRUPTIONSEED_HPP__
#define __CMP_CORRUPTIONSEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class CorruptionSeed : public BasePersistent<unsigned long>
{
public:
  CorruptionSeed( unsigned long value = 0 )
      : BasePersistent<unsigned long>( value )
  {
  }
  virtual std::string class_name() const override { return "CorruptionSeed"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_CORRUPTIONSEED_HPP__
