#ifndef __CMP_NPC_LERP_SPEED_HPP__
#define __CMP_NPC_LERP_SPEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class NpcLerpSpeed : public BasePersistent<float>
{
public:
  NpcLerpSpeed( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcLerpSpeed"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_NPC_LERP_SPEED_HPP__