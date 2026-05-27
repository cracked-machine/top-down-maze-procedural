#ifndef SRC_CMPS_PERSISTENT_GRAVEYARDPROCGENBIRTHTHRESHOLD_HPP_
#define SRC_CMPS_PERSISTENT_GRAVEYARDPROCGENBIRTHTHRESHOLD_HPP_

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

class GraveyardProcGenBirthThreshold : public BasePersistent<uint8_t>
{
public:
  GraveyardProcGenBirthThreshold( uint8_t value = 0.f, uint8_t min_value = 0.f, uint8_t max_value = 0.f )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  [[nodiscard]] std::string class_name() const override { return "GraveyardProcGenBirthThreshold"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_GRAVEYARDPROCGENBIRTHTHRESHOLD_HPP_
