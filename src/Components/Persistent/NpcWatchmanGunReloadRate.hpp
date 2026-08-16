#ifndef SRC_COMPONENTS_PERSISTENT_NPCWATCHMANGUNRELOADRATE_HPP__
#define SRC_COMPONENTS_PERSISTENT_NPCWATCHMANGUNRELOADRATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

class NpcWatchmanGunReloadRate : public BasePersistent<float>
{
public:
  NpcWatchmanGunReloadRate( float value = 0, float min_value = 0, float max_value = 0 )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  [[nodiscard]] std::string class_name() const override { return "NpcWatchmanGunReloadRate"; }
  [[nodiscard]] const std::string get_detail() const override { return detail; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_NPCWATCHMANGUNRELOADRATE_HPP__
