#ifndef SRC_COMPONENTS_EXIT_HPP__
#define SRC_COMPONENTS_EXIT_HPP__

namespace Game::Cmp
{

class Exit
{
public:
  // Exit spawns randomly at the start of game but is locked. It can be unlocked later.
  bool m_locked{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_EXIT_HPP__
