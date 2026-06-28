#ifndef SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENSURVIVALTHRESHOLD_HPP__
#define SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENSURVIVALTHRESHOLD_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

class GraveyardProcGenSurvivalThreshold : public BasePersistent<uint8_t>
{
public:
  GraveyardProcGenSurvivalThreshold( uint8_t value = 0.f, uint8_t min_value = 0.f, uint8_t max_value = 0.f )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  [[nodiscard]] std::string class_name() const override { return "GraveyardProcGenSurvivalThreshold"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_GRAVEYARDPROCGENSURVIVALTHRESHOLD_HPP__
