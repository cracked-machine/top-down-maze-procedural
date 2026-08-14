#ifndef SRC_COMPONENTS_SYSTEM_HPP__
#define SRC_COMPONENTS_SYSTEM_HPP__

namespace Game::Cmp
{

class System
{
public:
  bool level_complete = false;
  bool shaders_enabled = true; // F9
  bool particle_test_enabled = true;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_SYSTEM_HPP__
