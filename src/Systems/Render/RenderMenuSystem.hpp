#ifndef __SYS_RENDERMENUSYSTEM_HPP__
#define __SYS_RENDERMENUSYSTEM_HPP__

#include <Shaders/TitleScreenShader.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Constants.hpp>

#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Sprites
{
class TitleScreenShader;
} // namespace ProceduralMaze::Sprites

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
  ~RenderMenuSystem();

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  void render_title();

  void render_settings_widgets( sf::Time globalDeltaTime, sf::FloatRect title_text_dimensions );
  void render_settings( sf::Time globalDeltaTime );
  void render_paused( sf::Time globalDeltaTime );
  void render_defeat_screen();
  void render_victory_screen( bool allow_continue );

  //! @brief Initializes the shaders.
  //! @param display_res Dimensions for initializing internal shader textures
  void init_title_shaders( const Cmp::Persist::DisplayResolution &display_res );

private:
  // Lazy initialize the shaders - RenderMenuSystem::init_shaders() - once we know the screen resolution.
  std::unique_ptr<Sprites::TitleScreenShader> m_title_screen_shader;
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_RENDERMENUSYSTEM_HPP__