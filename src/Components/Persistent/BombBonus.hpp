#ifndef __CMP_BOMBBONUS_HPP__
#define __CMP_BOMBBONUS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>
namespace ProceduralMaze::Cmp::Persist
{

class BombBonus : public BasePersistent<uint8_t>
{
public:
  BombBonus( uint8_t value = 3, uint8_t min_value = 0, uint8_t max_value = 20 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "BombBonus"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_BOMBBONUS_HPP__