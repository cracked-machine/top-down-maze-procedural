#ifndef SRC_COMPONENTS_ABSOLUTEOFFSET_HPP__
#define SRC_COMPONENTS_ABSOLUTEOFFSET_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

//! @brief Component to store an absolute offset (in pixels), set by other systems.
class AbsoluteOffset
{
public:
  //! @brief Construct with an initial offset vector.
  //! @param offset Offset in pixels. Defaults to zero.
  AbsoluteOffset( const sf::Vector2f &offset = sf::Vector2f( 0.f, 0.f ) )
      : m_offset( offset )
  {
  }

  //! @brief Construct with initial offset components.
  //! @param x X offset in pixels.
  //! @param y Y offset in pixels.
  AbsoluteOffset( float x, float y )
      : m_offset( { x, y } )
  {
  }

  //! @brief Get the stored offset.
  //! @return const sf::Vector2f& Offset in pixels.
  const sf::Vector2f &getOffset() const { return m_offset; }

  //! @brief Set the stored offset.
  //! @param offset Offset in pixels.
  void setOffset( const sf::Vector2f &offset ) { m_offset = offset; }

  //! @brief Set the X component of the stored offset.
  //! @param x X offset in pixels.
  void setOffsetX( float x ) { m_offset.x = x; }

  //! @brief Set the Y component of the stored offset.
  //! @param y Y offset in pixels.
  void setOffsetY( float y ) { m_offset.y = y; }

private:
  //! @brief Offset in pixels.
  sf::Vector2f m_offset;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ABSOLUTEOFFSET_HPP__
