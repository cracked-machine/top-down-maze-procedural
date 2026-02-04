#ifndef __CMP_CORRUPTIONSEED_HPP__
#define __CMP_CORRUPTIONSEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class CorruptionSeed : public BasePersistent<uint64_t>
{
public:
  CorruptionSeed( uint64_t value = 0 )
      : BasePersistent<uint64_t>( value, 0, std::numeric_limits<uint64_t>::max() )
  {
  }
  virtual std::string class_name() const override { return "CorruptionSeed"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_CORRUPTIONSEED_HPP__
