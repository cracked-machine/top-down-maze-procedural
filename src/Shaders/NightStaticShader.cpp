#include <Components/Position.hpp>
#include <Constants.hpp>
#include <Optimizations.hpp>
#include <Persistent/DisplayResolution.hpp>
#include <Player/TorchRadius.hpp>
#include <Shaders/UniformBuilder.hpp>
#include <Systems/ParticleSystem.hpp>
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
  sf::Vector2f view_top_left = { view_center.x - ( view_size.x / 2.f ), view_center.y - ( view_size.y / 2.f ) };

  std::vector<sf::Vector2f> torch_positions;

  // add the radius for each flame particle sprite so the shader can render a circle of light
  for ( auto [candle_entt, particle_cmp] : reg.view<Sys::ParticleSpriteOwner>().each() )
  {
    if ( not particle_cmp.sprite->get_tag().contains( "candle" ) ) continue;
    torch_positions.push_back( particle_cmp.sprite->get_emitter_position() );
  }

  // flame particle sprites are paused when candle is in inventory so we need to add the radius explicitly
  auto [item_entt, item_type] = Utils::Player::get_inventory_type( reg );
  if ( item_type.contains( "candle" ) ) { torch_positions.push_back( Utils::Player::get_position( reg ).getCenter() ); }

  Sprites::UniformBuilder{}
      .set( "resolution", sf::Vector2f{ display_size } )
      .set( "time", elapsed().asSeconds() )
      .set( "view_top_left", view_top_left )
      .set( "view_size", view_size )
      .set( "torch_count", static_cast<int>( torch_positions.size() ) )
      .set( "torch_world_pos", torch_positions )
      .set( "torch_radius", Utils::Player::get_torch_radius( reg ).value )
      .apply( &get_shader() );

  // shader position at the top left of the world
  set_position( { 0, 0 } );
}

} // namespace ProceduralMaze::Sprites