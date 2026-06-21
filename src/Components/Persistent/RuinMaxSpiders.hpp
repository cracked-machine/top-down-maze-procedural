#ifndef SRC_CMPS_PERSISTENT_RUINMAXSPIDERS_HPP_
#define SRC_CMPS_PERSISTENT_RUINMAXSPIDERS_HPP_

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

class RuinMaxSpiders : public BasePersistent<uint16_t>
{
public:
  RuinMaxSpiders( uint16_t value = 0, uint16_t min_value = 0, uint16_t max_value = 0 )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }

  [[nodiscard]] std::string class_name() const override { return "RuinMaxSpiders"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_RUINMAXSPIDERS_HPP_