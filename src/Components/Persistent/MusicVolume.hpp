#ifndef __CMP_MUSICVOLUME_HPP__
#define __CMP_MUSICVOLUME_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class MusicVolume : public BasePersistent<float>
{
public:
  MusicVolume( float value = 20.0f, float min_value = 0.0f, float max_value = 100.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "MusicVolume"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_MUSICVOLUME_HPP__
