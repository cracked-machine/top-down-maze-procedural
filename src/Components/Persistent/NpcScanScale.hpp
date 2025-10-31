#ifndef __CMP_NPC_SCAN_SCALE_HPP__
#define __CMP_NPC_SCAN_SCALE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcScanScale : public BasePersistent<float>
{
public:
  NpcScanScale( float value = 2.5 )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "NpcScanScale"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_SCAN_SCALE_HPP__
