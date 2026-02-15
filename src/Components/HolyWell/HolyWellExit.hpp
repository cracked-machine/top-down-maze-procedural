#ifndef SRC_COMPONENTS_HOLYWELLEXIT_HPP__
#define SRC_COMPONENTS_HOLYWELLEXIT_HPP__

namespace ProceduralMaze::Cmp
{

// Used for collision detection on crypt doors
class HolyWellExit
{
public:
  HolyWellExit( bool is_open = false )
      : m_is_open( is_open )
  {
  }

  bool is_open() const { return m_is_open; }
  void set_is_open( bool open ) { m_is_open = open; }

private:
  // Is collision detection enabled for this sprite
  bool m_is_open{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_HOLYWELLEXIT_HPP__