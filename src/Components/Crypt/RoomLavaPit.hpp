#ifndef SRC_COMPONENTS_CRYPT_ROOMLAVAPIT_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMLAVAPIT_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp::Crypt
{

//! @brief Marks the lava pit hazard area within a Cmp::Crypt::RoomOpen, defined geometrically as a
//! rectangle spanning its Cmp::Crypt::RoomLavaPitCell parts.
class RoomLavaPit : public sf::FloatRect
{
public:
  //! @brief Construct a lava pit covering the given rectangle.
  //! @param bounds World-space bounds of the lava pit.
  RoomLavaPit( sf::FloatRect bounds )
      : sf::FloatRect( bounds ) {

        };
  //! @brief Construct a lava pit covering the given rectangle.
  //! @param position Top-left world position of the lava pit.
  //! @param size Pixel dimensions of the lava pit.
  RoomLavaPit( sf::Vector2f position, sf::Vector2f size )
      : sf::FloatRect( position, size ) {

        };

  //! @brief Get the world-space bounds of this lava pit.
  //! @return sf::FloatRect
  sf::FloatRect getBounds() const;

private:
  //! @brief World-space bounds of this lava pit.
  sf::FloatRect m_bounds;
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMLAVAPIT_HPP__
