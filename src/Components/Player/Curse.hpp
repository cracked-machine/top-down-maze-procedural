#ifndef SRC_COMPONENTS_PLAYER_CURSE_HPP__
#define SRC_COMPONENTS_PLAYER_CURSE_HPP__

#include <Components/Random.hpp>
namespace Game::Cmp::Player
{

struct Curse
{
  bool active{ false };

  //! @brief value that can be used for the shader alpha
  class ShaderAlpha
  {
  public:
    float add( float a )
    {
      if ( alpha < MAX ) { return alpha += a; }
      else { return alpha; }
    }
    float random()
    {
      Cmp::RandomFloat flicker( 0.f, 1.f );
      return alpha = flicker.gen();
    }
    float get() { return alpha; }
    void reset() { alpha = 0.f; }
    static constexpr float MAX = 0.75;

  private:
    float alpha = 0.f;
  };

  ShaderAlpha shader_alpha;
};
} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_CURSE_HPP__
