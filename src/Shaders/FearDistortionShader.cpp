#include <Components/Persistent/DisplayResolution.hpp>
#include <Shaders/UniformBuilder.hpp>
#include <Systems/PersistSystem.hpp>
#include <Utils/Player.hpp>

#include <Shaders/FearDistortionShader.hpp>

namespace Game::Sprites
{

void FearDistortionShader::update( entt::registry &reg )
{
  auto display_size = sf::Vector2f( Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg ) );
  float fear_amount = static_cast<float>( Utils::Player::get_player_stats( reg ).fear() ) / 100.f;

  Sprites::UniformBuilder{}.set( "resolution", display_size ).set( "time", elapsed().asSeconds() ).set( "fear", fear_amount ).apply( &get_shader() );

  set_position( { 0.f, 0.f } );
}

} // namespace Game::Sprites
