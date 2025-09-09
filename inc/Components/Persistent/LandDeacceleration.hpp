#ifndef __CMP_LANDDECELERATION_HPP__
#define __CMP_LANDDECELERATION_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class LandDeceleration
{
public:
  float &operator()() { return land_deceleration; }

private:
  float land_deceleration = 600.0f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_LANDDECELERATION_HPP__
