#include <Components/Position.hpp>
#include <Persistent/DisplayResolution.hpp>
#include <Shaders/UniformBuilder.hpp>
#include <Systems/PersistSystem.hpp>
#include <Utils/Player.hpp>

#include <Shaders/PulsingShader.hpp>

namespace ProceduralMaze::Sprites
{

void PulsingShader::update( entt::registry &reg )
{
  sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg );

  Sprites::UniformBuilder{}
      .set( "alpha", 0.5f )
      .set( "resolution", sf::Vector2f{ display_size } )
      .set( "time", elapsed().asSeconds() )
      .apply( &get_shader() );

  // clang-format on
  set_center_at_position( Utils::Player::get_position( reg ).position );
}

} // namespace ProceduralMaze::Sprites