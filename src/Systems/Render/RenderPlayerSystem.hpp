#ifndef __SYS_RENDERPLAYER_SYSTEM_HPP__
#define __SYS_RENDERPLAYER_SYSTEM_HPP__

#include <Components/RectBounds.hpp>
#include <Components/SpriteAnimation.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Angle.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
{

class RenderPlayerSystem : public RenderSystem
{
public:
  //! @brief
  //! @param spritekey
  //! @param position
  //! @param sprite index
  //! @param scale
  //! @param alpha
  //! @param origin
  //! @param angle
  using RenderParams = std::tuple<Sprites::SpriteMetaType, // sprite key
                                  sf::FloatRect,           // position
                                  int,                     // sprite index
                                  sf::Vector2f,            // scale
                                  uint8_t,                 // alpha
                                  sf::Vector2f,            // origin
                                  sf::Angle>;              // angle

  RenderPlayerSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                      Audio::SoundBank &sound_bank )
      : RenderSystem( reg, window, sprite_factory, sound_bank )
  {
    SPDLOG_DEBUG( "RenderPlayerSystem initialized" );
  }

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void render_player();
  void render_player_footsteps();
  RenderParams calc_alive_render_params( const Sprites::SpriteMetaType &sprite_type, const Cmp::Position &pos_cmp,
                                         const Cmp::Direction &direction, const Cmp::SpriteAnimation &anim_cmp,
                                         entt::entity entity );
  RenderParams calc_falling_render_params( const Sprites::SpriteMetaType &sprite_type, const Cmp::Position &pos_cmp,
                                           const Cmp::Direction &direction, const Cmp::SpriteAnimation &anim_cmp,
                                           Cmp::PlayerMortality &pc_mort_cmp );

  void render_npc();

private:
  void render_path_distance( const Cmp::RectBounds &bounds_cmp, const sf::Color &outline_color );

  uint8_t m_player_current_alpha{ 255 };

  sf::Angle m_player_current_angle{ sf::radians( 0.f ) };
  sf::Vector2f m_player_current_scale{ 1.f, 1.f };
  sf::Clock m_player_death_clock;
  sf::Time m_fall_speed{ sf::seconds( 0.05f ) };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERPLAYER_SYSTEM_HPP__