#ifndef __CMP_MUSICVOLUME_HPP__
#define __CMP_MUSICVOLUME_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class MusicVolume : public BasePersistent<float>
{
public:
  MusicVolume( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "MusicVolume"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_MUSICVOLUME_HPP__
