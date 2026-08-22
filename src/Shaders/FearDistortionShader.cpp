#include <Components/Persistent/DisplayResolution.hpp>
#include <Shaders/UniformBuilder.hpp>
#include <Systems/PersistSystem.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Player.hpp>

#include <Shaders/FearDistortionShader.hpp>

namespace Game::Sprites
{

namespace
{
// How many "e-foldings" per second the displayed fear closes the gap to the real stat by; see
// m_smoothed_fear in FearDistortionShader.hpp for why this exists.
constexpr float kFearSmoothingRate = 3.0f;
} // namespace

void FearDistortionShader::update( entt::registry &reg )
{
  auto display_size = sf::Vector2f( Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( reg ) );
  float target_fear = static_cast<float>( Utils::Player::get_player_stats( reg ).fear() ) / 100.f;

  sf::Time now = elapsed();
  float dt = ( now - m_last_fear_update ).asSeconds();
  m_last_fear_update = now;
  m_smoothed_fear = Utils::Maths::exp_decay( m_smoothed_fear, target_fear, kFearSmoothingRate, dt );

  Sprites::UniformBuilder{}.set( "resolution", display_size ).set( "time", now.asSeconds() ).set( "fear", m_smoothed_fear ).apply( &get_shader() );

  set_position( { 0.f, 0.f } );
}

} // namespace Game::Sprites
