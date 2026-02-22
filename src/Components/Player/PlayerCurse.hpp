#ifndef SRC_CMPS_PLAYER_PLAYERCURSE_HPP_
#define SRC_CMPS_PLAYER_PLAYERCURSE_HPP_

#include <Random.hpp>
namespace ProceduralMaze::Cmp
{

struct PlayerCurse
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
} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_PLAYER_PLAYERCURSE_HPP_