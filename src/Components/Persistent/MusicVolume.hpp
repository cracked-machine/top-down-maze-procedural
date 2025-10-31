#ifndef __CMP_MUSICVOLUME_HPP__
#define __CMP_MUSICVOLUME_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class MusicVolume : public BasePersistent<float>
{
public:
  MusicVolume( float value )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "MusicVolume"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_MUSICVOLUME_HPP__
