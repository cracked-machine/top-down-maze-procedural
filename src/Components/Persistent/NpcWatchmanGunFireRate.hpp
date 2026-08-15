#ifndef SRC_COMPONENTS_PERSISTENT_NPCWATCHMANGUNFIRERATE_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCWATCHMANGUNFIRERATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

class NpcWatchmanGunFireRate : public BasePersistent<float>
{
public:
  NpcWatchmanGunFireRate( float value = 0, float min_value = 0, float max_value = 0 )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  [[nodiscard]] std::string class_name() const override { return "NpcWatchmanGunFireRate"; }
  [[nodiscard]] const std::string get_detail() const override { return detail; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCWATCHMANGUNFIRERATE_HPP__
