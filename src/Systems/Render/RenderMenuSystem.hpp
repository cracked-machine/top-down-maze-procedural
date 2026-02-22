#ifndef __SYS_RENDERMENUSYSTEM_HPP__
#define __SYS_RENDERMENUSYSTEM_HPP__

#include <Constants.hpp>
#include <SFML/System/Time.hpp>

#include <Shaders/TitleScreenShader.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys
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
  ~RenderMenuSystem() = default;

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void init_title();
  void render_title();

  void render_settings_widgets( sf::Time globalDeltaTime, sf::FloatRect title_text_dimensions );
  void render_settings( sf::Time globalDeltaTime );
  void render_paused( sf::Time globalDeltaTime );
  void render_defeat_screen();
  void render_victory_screen();

private:
  // std::unique_ptr<Sprites::TitleScreenShader> m_title_screen_shader;
  Sprites::TitleScreenShader m_title_screen_shader{ "res/shaders/Generic.vert", "res/shaders/TitleScreen.frag", { 1u, 1u } };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERMENUSYSTEM_HPP__