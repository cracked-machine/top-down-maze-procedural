#ifndef SRC_UTILS_CARDINAL_HPP__
#define SRC_UTILS_CARDINAL_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Utils
{

//! @brief A cyclable cardinal direction (East/South/West/North) that doubles as its own
//! range-based-for iterator, so it can be stepped manually via operator++ or iterated with
//! `for ( auto c : Cardinal() )`.
class Cardinal
{
public:
  enum Value : int { East = 0, South = 1, West = 2, North = 3, End = 4 };

  constexpr Cardinal( Value value = East )
      : m_value( value )
  {
  }

  //! @brief Step to the next cardinal direction (wraps East -> South -> West -> North -> East)
  constexpr Cardinal &operator++()
  {
    m_value = static_cast<Value>( ( m_value + 1 ) % End );
    return *this;
  }

  constexpr bool operator==( const Cardinal &rhs ) const = default;

  //! @brief Dereference: an iterator over Cardinal yields Cardinal itself
  constexpr Cardinal operator*() const { return *this; }

  //! @brief Range support — enables `for ( auto c : Cardinal() )`
  static constexpr Cardinal begin() { return { East }; }
  static constexpr Cardinal end() { return { End }; }

  [[nodiscard]] constexpr Value value() const { return m_value; }

  //! @brief Convert to the corresponding unit vector: {1,0} right, {0,1} down, {-1,0} left, {0,-1} up
  [[nodiscard]] constexpr sf::Vector2f vector() const
  {
    switch ( m_value )
    {
      case East:
        return { 1.f, 0.f };
      case South:
        return { 0.f, 1.f };
      case West:
        return { -1.f, 0.f };
      case North:
        return { 0.f, -1.f };
      default:
        return { 0.f, 0.f };
    }
  }

private:
  Value m_value;
};

} // namespace Game::Utils

#endif // SRC_UTILS_CARDINAL_HPP__
