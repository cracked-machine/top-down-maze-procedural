
#include <Components/Position.hpp>
#include <Constants.hpp>
#include <Persistent/DisplayResolution.hpp>
#include <Shaders/PulsingShader.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Player.hpp>

#include <Shaders/DarkModeShader.hpp>

namespace ProceduralMaze::Sprites
{

void DarkModeShader::update( entt::registry &reg )
{
  auto display_res = sf::Vector2f( Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg ) );
  sf::Vector2f aperture_half_size( Constants::kGridSizePxF * 4.f );

  Sprites::UniformBuilder{}
      .set( "local_resolution", Sys::RenderSystem::get_world_view().getSize() )
      .set( "display_resolution", display_res )
      .set( "aperture_half_size", aperture_half_size )
      .set( "time", elapsed().asSeconds() )
      .apply( &get_shader() );

  set_center_at_position( Utils::Player::get_position( reg ).position );
}

} // namespace ProceduralMaze::Sprites