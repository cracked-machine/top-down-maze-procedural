#include <Components/Position.hpp>
#include <Constants.hpp>
#include <Persistent/DisplayResolution.hpp>
#include <Shaders/UniformBuilder.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Player.hpp>

#include <Shaders/NightStaticShader.hpp>

namespace ProceduralMaze::Sprites
{

void NightStaticShader::update( entt::registry &reg )
{
  sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg );
  sf::Vector2f view_center = Sys::RenderSystem::get_world_view().getCenter();
  sf::Vector2f view_size = Sys::RenderSystem::get_world_view().getSize();
  sf::Vector2f view_top_left = { view_center.x - view_size.x / 2.f, view_center.y - view_size.y / 2.f };

  // Check the src/Shaders/NightStaticShader.cpp::MAX_TORCH_COUNT before adding more to this container!
  std::vector torch_positions{ Utils::Player::get_position( reg ).getCenter(), { 900.0, 900.0 } };

  Sprites::UniformBuilder{}
      .set( "resolution", sf::Vector2f{ display_size } )
      .set( "time", elapsed().asSeconds() )
      .set( "view_top_left", view_top_left )
      .set( "view_size", view_size )
      .set( "torch_count", static_cast<int>( torch_positions.size() ) )
      .set( "torch_world_pos", torch_positions )
      .apply( &get_shader() );

  // shader position at the top left of the world
  set_position( { 0, 0 } );
}

} // namespace ProceduralMaze::Sprites