#ifndef SRC_COMPONENTS_CRYPT_CRYPTROOMLAVAPITCELL_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTROOMLAVAPITCELL_HPP__

#include <Components/Position.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

//! @brief Individual cells of the Cmp::CryptRoomLavaPit. Note render is acitvated by presence of Cmp::ZorderValue
class CryptRoomLavaPitCell : public Cmp::Position
{
public:
  CryptRoomLavaPitCell( const sf::Vector2f &pos, const sf::Vector2f &size )
      : Position( pos, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTROOMLAVAPITCELL_HPP__
