#ifndef SRC_COMPONENTS_PERSISTENT_SHOPMINPRICE_HPP__
#define SRC_COMPONENTS_PERSISTENT_SHOPMINPRICE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

//! @brief Minimum price the shop can charge for a generated item.
class ShopMinPrice : public BasePersistent<uint8_t>
{
public:
  //! @brief Construct with an initial price and optional min/max bounds.
  //! @param value initial price value
  //! @param min_value minimum allowed price value
  //! @param max_value maximum allowed price value
  ShopMinPrice( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }

  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "ShopMinPrice"
  [[nodiscard]] std::string class_name() const override { return "ShopMinPrice"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_SHOPMINPRICE_HPP__
