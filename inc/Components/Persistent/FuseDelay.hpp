#ifndef __CMP_FUSEDELAY_HPP__
#define __CMP_FUSEDELAY_HPP__

#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class FuseDelay
{
public:
  float &operator()() { return delay; }

private:
  float delay = 3.f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_FUSEDELAY_HPP__