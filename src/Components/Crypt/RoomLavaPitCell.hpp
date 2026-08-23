#ifndef SRC_COMPONENTS_CRYPT_ROOMLAVAPITCELL_HPP__
#define SRC_COMPONENTS_CRYPT_ROOMLAVAPITCELL_HPP__

#include <Components/Position.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp::Crypt
{

//! @brief Individual cells of the Cmp::Crypt::RoomLavaPit. Note render is acitvated by presence of Cmp::ZorderValue
class RoomLavaPitCell : public Cmp::Position
{
public:
  //! @brief Construct a new lava pit cell.
  //! @param pos World position of the cell.
  //! @param size Pixel dimensions of the cell.
  RoomLavaPitCell( const sf::Vector2f &pos, const sf::Vector2f &size )
      : Position( pos, size )
  {
  }
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ROOMLAVAPITCELL_HPP__
