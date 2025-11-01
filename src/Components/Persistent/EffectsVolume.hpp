#ifndef __CMP_EFFECTSVOLUME_HPP__
#define __CMP_EFFECTSVOLUME_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class EffectsVolume : public BasePersistent<float>
{
public:
  EffectsVolume( float value = 20.0f )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "EffectsVolume"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_EFFECTSVOLUME_HPP__
