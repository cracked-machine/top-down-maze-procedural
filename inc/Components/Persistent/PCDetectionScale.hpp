#ifndef __CMP_PC_DETECTION_SCALE_HPP__
#define __CMP_PC_DETECTION_SCALE_HPP__

#include <Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class PCDetectionScale : public BasePersistent<float>
{
public:
  PCDetectionScale() : BasePersistent<float>( 7.f ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PC_DETECTION_SCALE_HPP__