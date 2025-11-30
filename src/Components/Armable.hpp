#ifndef __CMP_ARMABLE_HPP__
#define __CMP_ARMABLE_HPP__

namespace ProceduralMaze::Cmp
{

//! @brief Component that indicates an entity can be armed by bomb
struct Armable
{
  [[maybe_unused]] bool armable{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_ARMABLE_HPP__