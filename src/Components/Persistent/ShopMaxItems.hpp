#ifndef SRC_CMPS_PERSISTENT_SHOPMAXITEMS_HPP_
#define SRC_CMPS_PERSISTENT_SHOPMAXITEMS_HPP_

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

class ShopMaxItems : public BasePersistent<uint8_t>
{
public:
  ShopMaxItems( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }

  [[nodiscard]] std::string class_name() const override { return "ShopMaxItems"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_MAXSHOPITEMS_HPP_