
#include <Constants.hpp>
#include <Persistent/DisplayResolution.hpp>
#include <Shaders/UniformBuilder.hpp>
#include <Systems/PersistSystem.hpp>

#include <Shaders/TitleScreenShader.hpp>

namespace ProceduralMaze::Sprites
{

void TitleScreenShader::update( entt::registry &reg )
{
  auto display_size = sf::Vector2f( Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg ) );
  const auto mouse_pos = sf::Vector2f( Constants::kFallbackDisplaySize );
  Sprites::UniformBuilder{}
      .set( "time", elapsed().asSeconds() )
      .set( "pixel_threshold", ( mouse_pos.x + mouse_pos.y ) / 30 )
      .set( "mouse_cursor", mouse_pos )
      .set( "resolution", display_size )
      .set( "time", elapsed().asSeconds() )
      .apply( &get_shader() );

  set_position( { 0, 0 } );
}

} // namespace ProceduralMaze::Sprites