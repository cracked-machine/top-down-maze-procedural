#include <Components/Crypt/CryptShuffleTimer.hpp>
#include <SFML/System/Time.hpp>
#include <Utils/Crypt.hpp>

namespace Game::Utils::Crypt
{

void reset_crypt_shuffle_timer( entt::registry &reg )
{
  for ( auto [timer_entt, timer_cmp] : reg.view<Cmp::CryptShuffleTimer>().each() )
  {
    timer_cmp.m_elapsed = sf::Time::Zero;
  }
}

void update_crypt_shuffle_timer( entt::registry &reg, sf::Time dt )
{
  for ( auto [timer_entt, timer_cmp] : reg.view<Cmp::CryptShuffleTimer>().each() )
  {
    timer_cmp.m_elapsed += dt;
  }
}

bool is_crypt_shuffle_timer_expired( entt::registry &reg )
{
  for ( auto [timer_entt, timer_cmp] : reg.view<Cmp::CryptShuffleTimer>().each() )
  {
    if ( timer_cmp.m_elapsed < timer_cmp.m_timeout ) return false;
  }
  return true;
}

} // namespace Game::Utils::Crypt