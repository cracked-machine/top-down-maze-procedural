#ifndef SRC_SYSTEMS_RENDER_RENDERMENUSYSTEM_HPP__
#define SRC_SYSTEMS_RENDER_RENDERMENUSYSTEM_HPP__

#include <Shaders/TitleScreenShader.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Constants.hpp>

#include <SFML/System/Time.hpp>

namespace Game::Sprites
{
class TitleScreenShader;
} // namespace Game::Sprites

namespace Game::Sys
{

class RenderMenuSystem : public RenderSystem
{
public:
  //! @brief Helper struct for display settings options
  struct DisplaySettings
  {
    //! @brief List of available display resolutions. Initialised in the cpp file.
    static const std::vector<sf::Vector2u> resolutions;

    //! @brief ImGui-compatible getter for display resolutions
    static bool get( [[maybe_unused]] void *data, int idx, const char **out_text )
    {
      if ( idx < 0 || idx >= static_cast<int>( resolutions.size() ) ) return false;
      static std::string str;
      str = std::to_string( resolutions[idx].x ) + "x" + std::to_string( resolutions[idx].y );
      *out_text = str.c_str();
      return true;
    }
  };

  RenderMenuSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : RenderSystem( reg, window, sprite_factory, sound_bank )
  {
  }
  ~RenderMenuSystem();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  void render_title();

  void render_settings_widgets( sf::Time dt, sf::FloatRect title_text_dimensions );
  void render_settings( sf::Time dt );
  void render_paused( sf::Time dt );
  void render_defeat_screen();
  void render_victory_screen( bool allow_continue );
};

} // namespace Game::Sys

#endif // SRC_SYSTEMS_RENDER_RENDERMENUSYSTEM_HPP__
