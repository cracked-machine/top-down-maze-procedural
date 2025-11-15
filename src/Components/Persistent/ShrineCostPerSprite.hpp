#ifndef __CMP_SHRINECOST_HPP__
#define __CMP_SHRINECOST_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

//! @brief Shrine consists of 4 sprites, so this is cost per sprite.
//!
class ShrineCostPerSprite : public BasePersistent<uint8_t>
{
public:
  ShrineCostPerSprite( uint8_t value = 1, uint8_t min_value = 0, uint8_t max_value = 4 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "ShrineCostPerSprite"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_SHRINECOST_HPP__
