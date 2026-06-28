#ifndef SRC_COMPONENTS_PERSISTENT_RUINMAXCOBWEBS_HPP__
#define SRC_COMPONENTS_PERSISTENT_RUINMAXCOBWEBS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

class RuinMaxCobwebs : public BasePersistent<uint16_t>
{
public:
  RuinMaxCobwebs( uint16_t value = 0, uint16_t min_value = 0, uint16_t max_value = 0 )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }

  [[nodiscard]] std::string class_name() const override { return "RuinMaxCobwebs"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_RUINMAXCOBWEBS_HPP__
