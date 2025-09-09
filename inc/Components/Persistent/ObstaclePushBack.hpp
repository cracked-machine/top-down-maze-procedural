#ifndef __CMP_OBSTACLEPUSHBACK_HPP__
#define __CMP_OBSTACLEPUSHBACK_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class ObstaclePushBack
{
public:
  float &operator()() { return value; }

private:
  float value = 1.1f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_OBSTACLEPUSHBACK_HPP__
