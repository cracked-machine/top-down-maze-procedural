#ifndef __CMP_WATERACCELERATION_HPP__
#define __CMP_WATERACCELERATION_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class WaterAcceleration
{
public:
  float &operator()() { return water_acceleration; }

private:
  float water_acceleration = 250.0f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WATERACCELERATION_HPP__
