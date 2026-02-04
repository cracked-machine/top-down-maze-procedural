#ifndef __CMP_SINKHOLESEED_HPP__
#define __CMP_SINKHOLESEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

// This component does not use JSON serialization as it is managed internally by the game engine.
class SinkholeSeed : public BasePersistent<uint64_t>
{
public:
  SinkholeSeed( uint64_t value = 0 )
      : BasePersistent<uint64_t>( value, 0, std::numeric_limits<uint64_t>::max() )
  {
  }
  virtual std::string class_name() const override { return "SinkholeSeed"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_SINKHOLESEED_HPP__
