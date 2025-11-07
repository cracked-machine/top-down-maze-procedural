#ifndef __CMP_PLAYERHEALTH_HPP__
#define __CMP_PLAYERHEALTH_HPP__

#include <cstdint>
namespace ProceduralMaze::Cmp {

class PlayerHealth
{
public:
  explicit PlayerHealth( uint8_t initial_health = 100 )
      : health( initial_health )
  {
  }

  // player health, signed, to prevent wrap-around
  int8_t health{ 100 };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_PLAYERHEALTH_HPP__