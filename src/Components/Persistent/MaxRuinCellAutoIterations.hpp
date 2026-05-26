#ifndef SRC_CMPS_PERSISTENT_MAXRUINCELLAUTOITERATIONS_HPP_
#define SRC_CMPS_PERSISTENT_MAXRUINCELLAUTOITERATIONS_HPP_

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

class MaxRuinCellAutoIterations : public BasePersistent<uint16_t>
{
public:
  MaxRuinCellAutoIterations( uint16_t value = 0, uint16_t min_value = 0, uint16_t max_value = 0 )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }

  [[nodiscard]] std::string class_name() const override { return "MaxRuinCellAutoIterations"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_MAXRUINCELLAUTOITERATIONS_HPP_