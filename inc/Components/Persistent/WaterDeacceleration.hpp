#ifndef __CMP_WATERDEACCELERATION_HPP__
#define __CMP_WATERDEACCELERATION_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class WaterDeceleration
{
public:
  float &operator()() { return water_deceleration; }

private:
  float water_deceleration = 90.0f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WATERDEACCELERATION_HPP__
