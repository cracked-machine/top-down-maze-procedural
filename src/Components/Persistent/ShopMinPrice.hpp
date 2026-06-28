#ifndef SRC_COMPONENTS_PERSISTENT_SHOPMINPRICE_HPP__
#define SRC_COMPONENTS_PERSISTENT_SHOPMINPRICE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

class ShopMinPrice : public BasePersistent<uint8_t>
{
public:
  ShopMinPrice( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }

  [[nodiscard]] std::string class_name() const override { return "ShopMinPrice"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_SHOPMINPRICE_HPP__
