#ifndef SRC_UTILS_CARDINAL_HPP__
#define SRC_UTILS_CARDINAL_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Utils
{

//! @brief A cyclable cardinal direction (East/South/West/North). Range-based-for iterable via
//! `for ( auto c : Cardinal() )` (visits each direction exactly once) and independently steppable
//! via operator++ (wraps forever: North -> East -> South -> ...). These two are deliberately
//! decoupled — the range iterator has its own non-wrapping counter — because a wrapping operator++
//! driving range termination would never reach an end sentinel and loop forever.
class Cardinal
{
public:
  enum Value : int { East = 0, South = 1, West = 2, North = 3 };
  static constexpr int Count = 4;

  constexpr Cardinal( Value value = East )
      : m_value( value )
  {
  }

  //! @brief Step to the next cardinal direction (wraps East -> South -> West -> North -> East)
  constexpr Cardinal &operator++()
  {
    m_value = static_cast<Value>( ( static_cast<int>( m_value ) + 1 ) % Count );
    return *this;
  }

  constexpr bool operator==( const Cardinal &rhs ) const = default;

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

  //! @brief Minimal iterator over the 4 Cardinal values, using a plain non-wrapping counter —
  //! independent of Cardinal::operator++'s cyclic semantics.
  class Iterator
  {
  public:
    constexpr explicit Iterator( int index )
        : m_index( index )
    {
    }
    constexpr Cardinal operator*() const { return { static_cast<Value>( m_index ) }; }
    constexpr Iterator &operator++()
    {
      ++m_index;
      return *this;
    }
    constexpr bool operator!=( const Iterator &rhs ) const { return m_index != rhs.m_index; }

  private:
    int m_index;
  };

  //! @brief Range support — enables `for ( auto c : Cardinal() )`
  static constexpr Iterator begin() { return Iterator( 0 ); }
  static constexpr Iterator end() { return Iterator( Count ); }

private:
  Value m_value;
};

} // namespace Game::Utils

#endif // SRC_UTILS_CARDINAL_HPP__
