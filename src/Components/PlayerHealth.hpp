#ifndef __CMP_PLAYERHEALTH_HPP__
#define __CMP_PLAYERHEALTH_HPP__

#include <cstdint>
namespace ProceduralMaze::Cmp {

class PlayerHealth
{
public:
  explicit PlayerHealth( uint32_t initial_health = 100 )
      : health( initial_health )
  {
  }

  // i32 to mitigate accidental underflow as much as possible
  int32_t health{ 100 };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_PLAYERHEALTH_HPP__