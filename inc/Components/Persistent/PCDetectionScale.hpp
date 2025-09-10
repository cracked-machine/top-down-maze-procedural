#ifndef __CMP_PC_DETECTION_SCALE_HPP__
#define __CMP_PC_DETECTION_SCALE_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class PCDetectionScale
{
public:
  float &operator()() { return value; }

private:
  float value{ 7.f };
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PC_DETECTION_SCALE_HPP__