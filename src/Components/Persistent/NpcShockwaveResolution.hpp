#ifndef SRC_COMPONENTS_PERSISTENT_NPCSHOCKWAVERESOLUTION_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCSHOCKWAVERESOLUTION_HPP__

#include <Components/Npc/NpcShockwave.hpp>
#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

class NpcShockwaveResolution : public BasePersistent<uint16_t>
{
public:
  NpcShockwaveResolution( uint16_t value = 0, uint16_t min_value = 0, uint16_t max_value = 0 )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcShockwaveResolution"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCSHOCKWAVERESOLUTION_HPP__
