#ifndef SRC_COMPONENTS_PLAYER_CURSE_HPP__
#define SRC_COMPONENTS_PLAYER_CURSE_HPP__

#include <Components/Random.hpp>
namespace Game::Cmp::Player
{

//! @brief Marks the player as cursed and carries the shader alpha state used to render the curse effect.
struct Curse
{
  //! @brief Whether the curse is currently active on the player.
  bool active{ false };

  //! @brief Value that can be used for the shader alpha.
  class ShaderAlpha
  {
  public:
    //! @brief Add to the current alpha, clamped to MAX.
    //! @param a amount to add
    //! @return float the resulting alpha value
    float add( float a )
    {
      if ( alpha < MAX ) { return alpha += a; }
      else { return alpha; }
    }
    //! @brief Set the alpha to a random value in [0, 1], producing a flicker effect.
    //! @return float the resulting alpha value
    float random()
    {
      Cmp::RandomFloat flicker( 0.f, 1.f );
      return alpha = flicker.gen();
    }
    //! @brief Get the current alpha value.
    //! @return float the current alpha value
    float get() { return alpha; }
    //! @brief Reset the alpha value back to zero.
    void reset() { alpha = 0.f; }
    //! @brief Upper bound the alpha value is clamped to by add().
    static constexpr float MAX = 0.75;

  private:
    //! @brief Current alpha value.
    float alpha = 0.f;
  };

  //! @brief Shader alpha state driving the curse visual effect.
  ShaderAlpha shader_alpha;
};
} // namespace Game::Cmp::Player

#endif // SRC_COMPONENTS_PLAYER_CURSE_HPP__
