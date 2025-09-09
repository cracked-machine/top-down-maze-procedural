#ifndef __CMP_LANDACCELERATION_HPP__
#define __CMP_LANDACCELERATION_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class LandAcceleration
{
public:
  float &operator()() { return land_acceleration; }

private:
  float land_acceleration = 500.0f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_LANDACCELERATION_HPP__
