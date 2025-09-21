#ifndef __CMP_SINKHOLE_HPP__
#define __CMP_SINKHOLE_HPP__

namespace ProceduralMaze::Cmp {

struct SinkHole
{
  // active sinkholes can cause adjacent obstacles to become sinkholes
  // inactive sinkholes are inner sinkholes and can be skipped by the spread algorithm
  bool active{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_SINKHOLE_HPP__