#ifndef SRC_CMPS_SHOP_SHOPINVENTORY_HPP__
#define SRC_CMPS_SHOP_SHOPINVENTORY_HPP__

#include <Constants.hpp>
#include <Position.hpp>
#include <Sprites/SpriteMetaType.hpp>

namespace ProceduralMaze::Cmp
{

class ShopInventory
{
public:
  struct Config
  {
    int max_items;
    int min_price;
    int max_price;
    sf::Vector2i ui_position;
    sf::Vector2i ui_size;
    uint32_t ui_fontsize;
    sf::Color ui_fontcolor;
    sf::Color ui_mainbgcolor;
    sf::Color ui_mainlinecolor;
    float ui_mainlinesize;
    sf::Color ui_slotbgcolor;
    sf::Color ui_slotlinecolor;
    float ui_slotlinesize;

    //! @brief Calculated from ui_size and max_items
    float slot_padding;

    std::pair<sf::Vector2i, sf::Vector2f> get_position() const
    {
      // clang-format off
      return { 
        ui_position, 
        sf::Vector2f{ static_cast<float>( ui_position.x ) * static_cast<float>( Constants::kGridSizePx.x ),
                      static_cast<float>( ui_position.y ) * static_cast<float>( Constants::kGridSizePx.y ) } 
      };
      // clang-format on
    }
    std::pair<sf::Vector2i, sf::Vector2f> get_size() const
    {
      // clang-format off
      return { 
        ui_size, 
        sf::Vector2f{ static_cast<float>( ui_size.x ) * static_cast<float>( Constants::kGridSizePx.x ),
                      static_cast<float>( ui_size.y ) * static_cast<float>( Constants::kGridSizePx.y ) } 
      };
      // clang-format on
    }
  };

  ShopInventory( Config config )
      : m_config( config )
  {
    // given N slots and X width, pre-compute the max padding allow between slots (+1 for begin and end).
    auto [_, ui_size_px] = m_config.get_size();
    float slot_width_sum = Constants::kGridSizePxF.x * m_config.max_items;
    float total_padding_remainder = ui_size_px.x - slot_width_sum;
    m_config.slot_padding = total_padding_remainder / ( m_config.max_items + 1 );
  }

  std::vector<std::pair<Sprites::SpriteMetaType, uint32_t>> m_slots;

  [[maybe_unused]] Config m_config;

  bool is_enabled{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_SHOP_SHOPINVENTORY_HPP__