#ifndef __CMP_PLAYERWEALTH_HPP__
#define __CMP_PLAYERWEALTH_HPP__

#include <cstdint>
namespace ProceduralMaze::Cmp
{

class PlayerWealth
{
public:
  explicit PlayerWealth( uint32_t initial_wealth = 0 )
      : wealth( initial_wealth )
  {
  }

  // i32 to mitigate accidental underflow as much as possible
  int32_t wealth{ 0 };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_PLAYERWEALTH_HPP__