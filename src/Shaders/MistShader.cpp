#include <Persistent/DisplayResolution.hpp>
#include <Shaders/MistShader.hpp>
#include <Shaders/UniformBuilder.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Sprites
{

void MistShader::update( entt::registry &reg )
{
  sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg );
  sf::Vector2f view_center = Sys::RenderSystem::get_world_view().getCenter();
  sf::Vector2f view_size = Sys::RenderSystem::get_world_view().getSize();
  sf::Vector2f view_top_left = { view_center.x - view_size.x / 2.f, view_center.y - view_size.y / 2.f };

  Sprites::UniformBuilder{}
      .set( "alpha", 0.75f )
      .set( "resolution", sf::Vector2f{ display_size } )
      .set( "viewTopLeft", view_top_left )
      .set( "viewSize", view_size )
      .set( "time", elapsed().asSeconds() )
      .apply( &get_shader() );

  set_position( { -100, -100 } );
}

} // namespace ProceduralMaze::Sprites