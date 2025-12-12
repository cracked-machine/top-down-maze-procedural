#ifndef __CMP_NPC_SCAN_SCALE_HPP__
#define __CMP_NPC_SCAN_SCALE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class NpcScanScale : public BasePersistent<float>
{
public:
  NpcScanScale( float value = 2.5, float min_value = 1.0f, float max_value = 3.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcScanScale"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_NPC_SCAN_SCALE_HPP__
