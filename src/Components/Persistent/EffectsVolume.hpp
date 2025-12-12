#ifndef __CMP_EFFECTSVOLUME_HPP__
#define __CMP_EFFECTSVOLUME_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class EffectsVolume : public BasePersistent<float>
{
public:
  EffectsVolume( float value = 20.0f, float min_value = 0.0f, float max_value = 100.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "EffectsVolume"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_EFFECTSVOLUME_HPP__
